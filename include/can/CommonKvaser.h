//
// Created by raffael on 28.08.25.
//

#ifndef COMMONKVASER_H
#define COMMONKVASER_H
#include <array>
#include <cstdint>

#include "CANDriver.h"
struct RawKvaserMessage {
    uint32_t busChannelID;
    uint32_t messageID;
    uint8_t  data[64];
    uint8_t  dlc;
    uint64_t timestamp;
    CANDriver *driver;

    // Tracing fields
    uint64_t trace_produced_ns = 0;
    uint64_t trace_seqno = 0;
};

#endif //COMMONKVASER_H
