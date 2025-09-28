#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
//#include <zlib.h>

#include "logging/influxDb.h"

// Minimal forward declaration for tracing (avoid pulling in C++ header)
#if defined(HAVE_LTTNG) && HAVE_LTTNG
void __tracepoint_llserver___net_send_header(uint32_t bytes, uint64_t io_ns);
void __tracepoint_llserver___net_send_header_error(uint32_t attempted_bytes, uint64_t io_ns, int32_t err_no);
void __tracepoint_llserver___net_send_body(uint32_t bytes, uint64_t io_ns);
void __tracepoint_llserver___net_send_body_error(uint32_t attempted_bytes, uint64_t io_ns, int32_t err_no);
void __tracepoint_llserver___net_recv(uint32_t bytes, uint64_t io_ns);
void __tracepoint_llserver___net_recv_error(uint32_t attempted_bytes, uint64_t io_ns, int32_t err_no);
#endif

static void safe_str_cpy(char *src, const char *dest, size_t length) {
    strncpy(src, dest, length-1);
    src[length-1] = '\0';
}

// Monotonic time helper (nanoseconds)
static inline uint64_t monotonic_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;
}

// LoggingOp::NET_SEND op code from tracepoint_wrapper.h (enum value = 1)
#define LLSERVER_LOGGING_OP_NET_SEND 1
#define LLSERVER_LOGGING_OP_NET_RECV 4

void set_timestamp_precision(influxDbContext *cntxt, timestamp_precision_t precision) {
    switch(precision) {
        case MICROSECONDS:
            strncpy(cntxt->ts_precision, "&precision=u", SETTINGS_LENGTH);
        break;
        case MILLISECONDS: 
            strncpy(cntxt->ts_precision, "&precision=ms", SETTINGS_LENGTH);
        break;
        case SECONDS:
            strncpy(cntxt->ts_precision, "&precision=s", SETTINGS_LENGTH);
        break;
        case MINUTES:
            strncpy(cntxt->ts_precision, "&precision=m", SETTINGS_LENGTH);
        break;
        case HOURS:
            strncpy(cntxt->ts_precision, "&precision=h", SETTINGS_LENGTH);
        break;
        default: 
            strncpy(cntxt->ts_precision, "", SETTINGS_LENGTH);
        break;
    }
}

void set_credentials(influxDbContext *cntxt, const char *username, const char *password) {
    safe_str_cpy(cntxt->user, username, SETTINGS_LENGTH);
    safe_str_cpy(cntxt->password, password, SETTINGS_LENGTH);
}

int initDbContext(influxDbContext *cntxt, const char *hostname, const char *port, const char *database) {
    struct addrinfo hints, *ai = NULL;

    cntxt->sock_fd = -1;
    memset(cntxt->user, 0, SETTINGS_LENGTH);
    memset(cntxt->password, 0, SETTINGS_LENGTH);
    memset(cntxt->ts_precision, 0, SETTINGS_LENGTH);

    safe_str_cpy(cntxt->hostname, hostname, SETTINGS_LENGTH);
    safe_str_cpy(cntxt->port, port, SETTINGS_LENGTH);
    safe_str_cpy(cntxt->db_name, database, SETTINGS_LENGTH);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    do {
        if(getaddrinfo(hostname, port, &hints, &ai) != 0) {
            break;
        }

        cntxt->sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if(cntxt->sock_fd < 0) {
            break;
        }

        if(connect(cntxt->sock_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
            (void) close(cntxt->sock_fd);
            cntxt->sock_fd = -1;
        }
    }
    while (0);

    freeaddrinfo(ai);

    return cntxt->sock_fd;
}

int deInitDbContext(influxDbContext *cntxt) {
    return close(cntxt->sock_fd);
}


int sendData(influxDbContext *cntxt, char *data, size_t length) {
    char http_header[2048];
    ssize_t ret; // changed from size_t to ssize_t for proper error detection
    size_t header_length = 0, sent = 0;
    char result[1024];

    header_length += snprintf(http_header, 2048, "POST /write?db=%s%s%s%s HTTP/1.1\r\nHost: %s:%s\r\nContent-Length: %zd\r\n\r\n",
            cntxt->db_name, cntxt->user, cntxt->password, cntxt->ts_precision, cntxt->hostname, cntxt->port, length);

    while (sent < header_length) {
        uint64_t t0 = monotonic_ns();
        ret = write(cntxt->sock_fd, &http_header[sent], header_length - sent);
        uint64_t t1 = monotonic_ns();
#if defined(HAVE_LTTNG) && HAVE_LTTNG
        if (ret < 0) {
            __tracepoint_llserver___net_send_header_error((uint32_t)(header_length - sent), t1 - t0, errno);
        } else if (ret > 0) {
            __tracepoint_llserver___net_send_header((uint32_t)ret, t1 - t0);
        }
#endif
        if (ret < 0) {
            return -1;
        }
        sent += (size_t)ret;
    }
    sent = 0;

    while (sent < length) {
        uint64_t t0 = monotonic_ns();
        ret = write(cntxt->sock_fd, &data[sent], length - sent);
        uint64_t t1 = monotonic_ns();
#if defined(HAVE_LTTNG) && HAVE_LTTNG
        if (ret < 0) {
            __tracepoint_llserver___net_send_body_error((uint32_t)(length - sent), t1 - t0, errno);
        } else if (ret > 0) {
            __tracepoint_llserver___net_send_body((uint32_t)ret, t1 - t0);
        }
#endif
        if (ret < 0) {
            return -1;
        }
        sent += (size_t)ret;
    }

    // If needed -> extract response code (DB)
    uint64_t t0r = monotonic_ns();
    ssize_t rret = read(cntxt->sock_fd, result, sizeof(result));
    uint64_t t1r = monotonic_ns();
#if defined(HAVE_LTTNG) && HAVE_LTTNG
    if (rret < 0) {
        __tracepoint_llserver___net_recv_error((uint32_t)sizeof(result), t1r - t0r, errno);
    } else if (rret > 0) {
        __tracepoint_llserver___net_recv((uint32_t)rret, t1r - t0r);
    }
#endif
    (void) rret; // suppress unused warning if LTTng disabled
    // fprintf(stdout, "Result %s\n", result);

    return 0;
}
