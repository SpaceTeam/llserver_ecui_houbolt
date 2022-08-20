//
// Created by Christofer Held on 13.08.22.
//
#include <gtest/gtest.h>
#include "utility/RingBuffer.h"

TEST(WritingAndReadingFromRingBuffer, ReadingAndWriting) {
  RingBuffer buffer = RingBuffer<int>();
  buffer.push(1);
  EXPECT_EQ(buffer.pop(), 1);
}

TEST(WritingAndReadingFromRingBuffer, ReadTimeoutShouldThrowException) {
  RingBuffer buffer = RingBuffer<int>();
  EXPECT_ANY_THROW(buffer.pop());
}

TEST(WritingAndReadingFromRingBuffer, WriteTimeoutShouldThrowException) {
  RingBuffer buffer = RingBuffer<int>();
  for(int i = 0; i < RingBuffer<int>::RING_BUFFER_SIZE; i++){
    buffer.push(i);
  }
  EXPECT_ANY_THROW(buffer.push(0));
}

TEST(WritingAndReadingFromRingBuffer, WriteFullReadAllShouldBeInOrder) {
  RingBuffer buffer = RingBuffer<int>();
  for(int i = 0; i < RingBuffer<int>::RING_BUFFER_SIZE; i++){
    buffer.push(i);
  }
  for(int i = 0; i < RingBuffer<int>::RING_BUFFER_SIZE; i++){
    EXPECT_EQ(buffer.pop(),i);
  }
}