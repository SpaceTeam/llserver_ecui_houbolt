//
// Created by Christofer Held on 13.08.22.
//
#include <gtest/gtest.h>
#include "utility/RingBuffer.h"

TEST(WritingAndReadingFromRingBuffer, ReadingAndWriting) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	buffer.push(1);

	EXPECT_EQ(buffer.pop(), 1);
}

TEST(WritingAndReadingFromRingBuffer, ReadTimeout_shouldThrowException) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();
	
	EXPECT_ANY_THROW(buffer.pop());
}

TEST(WritingAndReadingFromRingBuffer, WriteTimeout_shouldThrowException) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();
	for(int i = 0; i < size; i++){
		buffer.push(i);
	}
	EXPECT_ANY_THROW(buffer.push(0));
}

TEST(WritingAndReadingFromRingBuffer, WriteFullReadAll_shouldBeInOrder) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	for(int i = 0; i < size; i++){
		buffer.push(i);
	}

	for(int i = 0; i < size; i++){
		EXPECT_EQ(buffer.pop(),i);
	}
}

