//
// Created by Christofer Held on 13.08.22.
//
#include <gtest/gtest.h>
#include "utility/RingBuffer.h"

// Demonstrate some basic assertions.
TEST(WritingAndReadingFromRingBuffer, ReadingAndWriting) {
  RingBuffer r = RingBuffer<int>();
  r.push(1);
  EXPECT_EQ(r.pop(),1);
}

TEST(WritingAndReadingFromRingBuffer, ReadTimeoutShouldThrowException) {
  RingBuffer r = RingBuffer<int>();
  EXPECT_ANY_THROW(r.pop());
}

TEST(WritingAndReadingFromRingBuffer, WriteTimeoutShouldThrowException) {
  RingBuffer r = RingBuffer<int>();
  for(int i = 0; i < r.RING_BUFFER_SIZE; i++){
    r.push(i);
  }
  EXPECT_ANY_THROW(r.push(0));
}