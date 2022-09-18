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


TEST(WritingAndReadingFromRingBuffer, PopAllInOrder_shouldBeInOrder) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	for(size_t i = 0; i < 30; i++) {
		buffer.push(i);
	}

	for (size_t i = 0;i < 5; i++) {
		buffer.pop();
	}

	std::pair<int[32], size_t> test = buffer.pop_all();

	EXPECT_EQ(test.second, 25);

	for (size_t i = 0;i < test.second; i++) {
		EXPECT_EQ(test.first[i], 5 + i);
	}
}


TEST(WritingAndReadingFromRingBuffer, PopAllOverBorder_shouldBeInOrder) {
	const int size = 32;

	RingBuffer buffer = RingBuffer<int, size>();

	for(size_t i = 0; i < 32; i++) {
		buffer.push(i);
	}

	for (size_t i = 0;i < 5; i++) {
		buffer.pop();
	}

	for(size_t i = 32; i < 35; i++) {
		buffer.push(i);
	}

	std::pair<int[32], size_t> test = buffer.pop_all();

	EXPECT_EQ(test.second, 30);

	for (size_t i = 0;i < test.second; i++) {
		EXPECT_EQ(test.first[i], 5 + i);
	}
}

