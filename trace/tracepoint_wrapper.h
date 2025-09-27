#pragma once

#if defined(HAVE_LTTNG) && HAVE_LTTNG
extern "C" {
#  include "tracepoint.tp"
}
#else
// Fallback: compile away
#  define tracepoint(provider, name, ...) ((void)0)
#endif