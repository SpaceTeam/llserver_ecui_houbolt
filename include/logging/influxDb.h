#ifndef INFLUXDB_H
#define INFLUXDB_H

#define SETTINGS_LENGTH 256

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef enum {
        NANOSECONDS,
        MICROSECONDS,
        MILLISECONDS,
        SECONDS,
        MINUTES,
        HOURS
    } timestamp_precision_t;

    typedef struct {
        char hostname[SETTINGS_LENGTH];
        char port[SETTINGS_LENGTH];
        char db_name[SETTINGS_LENGTH];
        char user[SETTINGS_LENGTH];
        char password[SETTINGS_LENGTH];
        int sock_fd;
        char ts_precision[SETTINGS_LENGTH];
    } influxDbContext;

    void set_timestamp_precision(influxDbContext *cntxt, timestamp_precision_t precision);
    void set_credentials(influxDbContext *cntxt, const char *username, const char *password);

    int initDbContext(influxDbContext *cntxt, const char *hostname, const char *port, const char *database);
    int deInitDbContext(influxDbContext *cntxt);

    int createSocket(influxDbContext *cntxt);

    int sendData(influxDbContext *cntxt, char *data, size_t length);
#ifdef __cplusplus
}
#endif
#endif