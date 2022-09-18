#ifndef CONTROL_FLAG
#define CONTROL_FLAG

#include <csignal>

extern sig_atomic_t volatile finished;
extern sig_atomic_t volatile log_peripherie_data;

#endif /* CONTROL_FLAG */
