//
// Created by Christofer Held on 13.08.22.
//
#include <gtest/gtest.h>

#include <optional>

#include "utility/RingBuffer.h"

TEST(WritingAndReadingFromRingBuffer, ReadingAndWriting) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	buffer.push(1);

	EXPECT_EQ(buffer.pop(), 1);
}

TEST(WritingAndReadingFromRingBuffer, NothingToRead_shouldReturnEmptyOptional) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	EXPECT_EQ(buffer.pop(), std::nullopt);
}

TEST(WritingAndReadingFromRingBuffer, NoSpaceToWrite_shouldReturnFalse) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	for(int i = 0; i < size; i++) {
		buffer.push(i);
	}

	EXPECT_FALSE(buffer.push(0));
}

TEST(WritingAndReadingFromRingBuffer, WriteFullReadAll_shouldBeInOrder) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	for(int i = 0; i < size; i++){
		buffer.push(i);
	}

	for(int i = 0; i < size; i++){
		EXPECT_EQ(buffer.pop(), i);
	}
}

