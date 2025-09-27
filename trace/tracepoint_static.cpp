#include "tracepoint_wrapper.h"

namespace llserver {
namespace trace {
    std::atomic<uint32_t> ConditionalTracer::sample_counter_{0};
}
}
