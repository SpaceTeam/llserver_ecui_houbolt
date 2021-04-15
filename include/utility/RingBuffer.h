//
// Created by Markus on 15.04.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_RINGBUFFER_H
#define LLSERVER_ECUI_HOUBOLT_RINGBUFFER_H

template <typename T>
class RingBuffer
{
private:
    T ringBuffer;
    T *head;
    T *tail;
    T *readPtr;
    T *writePtr;
};

#endif //LLSERVER_ECUI_HOUBOLT_RINGBUFFER_H
