#ifndef CONTROL_FLAG
#define CONTROL_FLAG

#include <signal.h>

extern volatile sig_atomic_t finished;
extern volatile sig_atomic_t log_peripherie_data;

#endif /* CONTROL_FLAG */
