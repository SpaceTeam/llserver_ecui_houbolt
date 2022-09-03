#ifndef CONTROL_FLAG
#define CONTROL_FLAG

#include <signal.h>

#define LOGGING_FILE_PATH "logs/log.txt"

extern volatile sig_atomic_t finished;

const extern volatile bool logging_in_influx = true;
const extern volatile bool logging_in_file = true;

#endif /* CONTROL_FLAG */
