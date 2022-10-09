#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "semaphore.h"

#include <mutex>
#include <atomic>
#include <optional>
#include <cstring>
#include <array>
#include <algorithm>

/**
 * Multi producer, multi consumer ring buffer as a central repository of relevant data.
 * Multiple threads can read and write without race-conditions.
 * This class is thread-safe (critical section).
 */
template<typename ElementType, int ring_buffer_size = 32, bool input_blocking = false, bool output_blocking = false>
class RingBuffer {
private:
	ElementType buffer[ring_buffer_size];

	// Pointer for reading and writing position
	ElementType *read_pointer = buffer;
	ElementType *write_pointer = buffer;

	// Locks for the critical read and write section.
	std::mutex mutable write_mutex;
	std::mutex mutable read_mutex;

	// Semaphore and counter for reading and writing elements.
	std::atomic<size_t> unread_elements = 0;
	std::atomic<size_t> unwritten_elements = ring_buffer_size;

	sem_t unread_elements_semaphore;
	sem_t unwritten_elements_semaphore;

public:
	RingBuffer(void);
	~RingBuffer(void);

	// non copyable
	RingBuffer(RingBuffer const &) = delete;
	void operator=(RingBuffer const &) = delete;

	// movable
	RingBuffer(RingBuffer &&) = default;
	RingBuffer& operator=(RingBuffer &&) = default;

	bool push(ElementType);
	std::optional<ElementType> pop(void);

	bool push_all(std::pair<std::array<ElementType, ring_buffer_size>, size_t> const);
	std::pair<std::array<ElementType, ring_buffer_size>, size_t> pop_all(void);
};


template<typename ElementType, int ring_buffer_size, bool input_blocking, bool output_blocking>
RingBuffer<ElementType, ring_buffer_size, input_blocking, output_blocking>::RingBuffer(
	void
) {
	sem_init(&unread_elements_semaphore, 0, 0);
	sem_init(&unwritten_elements_semaphore, 0, ring_buffer_size);
}


template<typename ElementType, int ring_buffer_size, bool input_blocking, bool output_blocking>
RingBuffer<ElementType, ring_buffer_size, input_blocking, output_blocking>::~RingBuffer(
	void
) {
	sem_destroy(&unread_elements_semaphore);
	sem_destroy(&unwritten_elements_semaphore);
}


/**
 * Thread-safe method for writing and adding an element to the ringbuffer.
 * returns false if the ringbuffer is full and the element was not inserted.
 * The writing is critical, only one thread can write at a time.
 */
template<typename ElementType, int ring_buffer_size, bool input_blocking, bool output_blocking>
bool
RingBuffer<ElementType, ring_buffer_size, input_blocking, output_blocking>::push(
	ElementType value
) {
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	int error;

	// if possible decrease counter of unwritten elements else return failure of doing so
	if constexpr (input_blocking) {
		struct timespec waiting_time = {.tv_sec = 0, .tv_nsec = 200'000'000};
		error = sem_timedwait(&unwritten_elements_semaphore, &waiting_time);

	} else {
		error = sem_trywait(&unwritten_elements_semaphore);
	}
	if (error == -1) {
		return false;
	}
	unwritten_elements--;

	*write_pointer = value;

	// set write_pointer to next element
	write_pointer = (write_pointer == &buffer[ring_buffer_size - 1]) ? buffer : write_pointer + 1;

	// increase counter of unread elements
	unread_elements++;
	sem_post(&unread_elements_semaphore);

	return true;
}


/**
 * Thread-safe method for reading and removing an element to the ringbuffer.
 * returns empty optional if ringbuffer is empty and the element was not removed.
 * The reading is critical only one thread can read at a time.
 */
template<typename ElementType, int ring_buffer_size, bool input_blocking, bool output_blocking>
std::optional<ElementType>
RingBuffer<ElementType, ring_buffer_size, input_blocking, output_blocking>::pop(
	void
) {
	// critical read section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};

	int error;

	// if possible decrease counter of unread elements else return failure of doing so
	if constexpr (output_blocking) {
		struct timespec waiting_time = {.tv_sec = 0, .tv_nsec = 200'000'000};
		error = sem_timedwait(&unread_elements_semaphore, &waiting_time);

	} else {
		error = sem_trywait(&unread_elements_semaphore);
	}
	if (error == -1) {
		return std::nullopt;
	}
	unread_elements--;

	ElementType value = *read_pointer;

	// set read_pointer to next element
	read_pointer = (read_pointer == &buffer[ring_buffer_size - 1]) ? buffer : read_pointer + 1;

	// increase counter of written elements
	unwritten_elements++;
	sem_post(&unwritten_elements_semaphore);

	return std::make_optional(value);
}

inline
int
sem_timedwaitn(
	sem_t *mutex,
	uint32_t count,
	struct timespec * waiting_time
) {
	int error;

	uint32_t i;
	for (i = 0; i < count; i++) {
		error = sem_timedwait(mutex, waiting_time);
		if (error == -1) {
			break;
		}
	}

	if (error == -1) {
		// NOTE(Lukas Karafiat): if an interrupt happened we have to give the elements back
		for (uint32_t j = 0; j < i; j++) {
			sem_post(mutex);
		}

		return -1;
	}

	return 0;
}


inline
int
sem_trywaitn(
	sem_t *mutex,
	uint32_t count
) {
	int error;

	uint32_t i;
	for (i = 0; i < count; i++) {
		error = sem_trywait(mutex);
		if (error == -1) {
			break;
		}
	}

	if (error == -1) {
		// NOTE(Lukas Karafiat): if an interrupt happened we have to give the elements back
		for (uint32_t j = 0; j < i; j++) {
			sem_post(mutex);
		}

		return -1;
	}

	return 0;
}


inline
int
sem_postn(
	sem_t *mutex,
	size_t count
) {
	int error;

	for (size_t i = 0; i < count; i++) {
		error = sem_post(mutex);
	}

	return error;
}


template<typename ElementType, int ring_buffer_size, bool input_blocking, bool output_blocking>
bool
RingBuffer<ElementType, ring_buffer_size, input_blocking, output_blocking>::push_all(
	std::pair<std::array<ElementType, ring_buffer_size>, size_t> const values
) {
	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};

	int error;

	// if possible decrease counter of unwritten elements else return failure of doing so
	if constexpr (input_blocking) {
		// NOTE(Lukas Karafiat): we are the only thread in here and can acquire all elements in peace
		struct timespec waiting_time = {.tv_sec = 0, .tv_nsec = 200'000'000};
		error = sem_timedwaitn(&unwritten_elements_semaphore, values.second, &waiting_time);

	} else {
		if (unwritten_elements < values.second) {
			return false;
		}
		// NOTE(Lukas Karafiat): this should never fail to decrement as we checked counter that is synchronized with semaphore
		error = sem_trywaitn(&unwritten_elements_semaphore, values.second);
	}
	if (error == -1) {
		return false;
	}
	unwritten_elements -= values.second;

	// amount of elements before buffer wrap
	size_t upfront_element_count = ((buffer + ring_buffer_size) - write_pointer);

	if (values.second <= upfront_element_count) {
		std::copy_n(values.first.begin(), values.second, write_pointer);

		write_pointer = write_pointer + values.second;

	} else {
		std::copy_n(values.first.begin(), upfront_element_count, write_pointer);
		std::copy_n(values.first.begin() + upfront_element_count, values.second - upfront_element_count, buffer);

		write_pointer = buffer + (values.second - upfront_element_count);
	}

	// increase counter of unread elements
	unread_elements += values.second;
	sem_postn(&unread_elements_semaphore, values.second);

	return true;
}


template<typename ElementType, int ring_buffer_size, bool input_blocking, bool output_blocking>
std::pair<std::array<ElementType, ring_buffer_size>, size_t>
RingBuffer<ElementType, ring_buffer_size, input_blocking, output_blocking>::pop_all(
	void
) {
	// critical read section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};

	std::pair<std::array<ElementType, ring_buffer_size>, size_t> values{};
	values.second = 0;

	int error;

	// if possible decrease counter of unread elements else return failure of doing so
	if constexpr (output_blocking) {
		// NOTE(Lukas Karafiat): we are the only thread in here and can acquire all elements in peace
		struct timespec waiting_time = {.tv_sec = 0, .tv_nsec = 200'000'000};
		error = sem_timedwaitn(&unread_elements_semaphore, unread_elements, &waiting_time);

	} else {
		if (unread_elements <= 0) {
			return values;
		}
		// NOTE(Lukas Karafiat): this should never fail as we checked counter that is synchronized with semaphore
		error = sem_trywaitn(&unread_elements_semaphore, unread_elements);
	}
	if (error == -1) {
		return values;
	}
	unread_elements -= values.second;

	// amount of elements before buffer wrap
	size_t upfront_element_count = ((buffer + ring_buffer_size) - read_pointer);

	if (values.second <= upfront_element_count) {
		std::copy_n(read_pointer, values.second, values.first.begin());

		read_pointer = read_pointer + values.second;

	} else {
		std::copy_n(read_pointer, upfront_element_count, values.first.begin());
		std::copy_n(buffer, values.second - upfront_element_count, values.first.begin() + upfront_element_count);

		read_pointer = buffer + (values.second - upfront_element_count);
	}

	// increase counter of unwritten elements
	unwritten_elements += values.second;
	sem_postn(&unwritten_elements_semaphore, values.second);

	return values;
}

#endif /* QUEUE_HPP */
