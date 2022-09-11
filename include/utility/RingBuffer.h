#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <mutex>
#include <semaphore>
#include <optional>

/**
 * Multi producer, multi consumer ring buffer as a central repository of relevant data.
 * Multiple threads can read and write without race-conditions.
 * This class is thread-safe (critical section).
 */
template<typename ElementType, int ring_buffer_size = 32>
class RingBuffer {
private:
	ElementType buffer[ring_buffer_size];

	// Pointer for reading and writing position
	ElementType *read_pointer = buffer;
	ElementType *write_pointer = buffer;

	// Locks for the critical read and write section.
	mutable std::mutex write_mutex;
	mutable std::mutex read_mutex;

	// Semaphore for reading and writing elements.
	std::counting_semaphore<ring_buffer_size> unread_elements{0};
	std::counting_semaphore<ring_buffer_size> unwritten_elements{ring_buffer_size};

public:
	RingBuffer(void) = default;
	~RingBuffer(void) = default;

	// non copyable
	RingBuffer(const RingBuffer &x) = delete;
	void operator=(const RingBuffer &x) = delete;

	// movable
	RingBuffer(RingBuffer &&x) = default;
	RingBuffer& operator=(RingBuffer &&x) = default;

	bool push(ElementType value);
	std::optional<ElementType> pop(void);
};


/**
 * Thread-safe method for adding an element to the ringbuffer.
 * returns false if the ringbuffer is full and the element was not inserted.
 * The writing is critical, only one thread can write at a time.
 */
template<typename ElementType, int ring_buffer_size>
bool
RingBuffer<ElementType, ring_buffer_size>::push(
	ElementType value
) {
	if (!unwritten_elements.try_acquire()) {
		return false;
	}

	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};
	*write_pointer = value;

	// set write_pointer to next element
	write_pointer = (write_pointer == &buffer[ring_buffer_size - 1]) ? buffer : write_pointer + 1;

	// increase counter of unread elements
	unread_elements.release();

	return true;
}


/**
 * Thread-safe method for reading and removing an element from the queue.
 * returns empty optional if queue is empty.
 * The reading is critical only one thread can read at a time.
 */
template<typename ElementType, int ring_buffer_size>
std::optional<ElementType>
RingBuffer<ElementType, ring_buffer_size>::pop(
	void
) {
	if (!unread_elements.try_acquire()) {
		return std::nullopt;
	}

	// critical write section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};
	ElementType value = *read_pointer;

	// set read_pointer to next element
	read_pointer = (read_pointer == &buffer[ring_buffer_size - 1]) ? buffer : read_pointer + 1;

	// increase counter of unread elements
	unwritten_elements.release();

	return std::make_optional(value);
}

#endif /* QUEUE_HPP */
