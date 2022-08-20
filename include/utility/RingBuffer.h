#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <mutex>
#include <semaphore>

/**
 * Multi producer, multi consumer ring buffer as a central repository of relevant data.
 * Multiple threads can read and write without race-conditions.
 * This class is thread-safe (critical section).
 */
template<typename ElementType>
class RingBuffer {

public:
	// Variables for setup
	const static int RING_BUFFER_SIZE = 128;
	const std::chrono::milliseconds TIMEOUT{1000};

 private:
	ElementType buffer[RING_BUFFER_SIZE];

	// Pointer for reading and writing position
	ElementType *read_pointer = buffer;
	ElementType *write_pointer = buffer;

	// Locks for the critical read and write section.
	std::mutex write_mutex;
	std::mutex read_mutex;

	// Semaphore for reading and writing elements.
	std::counting_semaphore<RING_BUFFER_SIZE> unread_elements{0};
	std::counting_semaphore<RING_BUFFER_SIZE> unwritten_elements{RING_BUFFER_SIZE};

public:

	RingBuffer(void) = default;
	~RingBuffer(void) = default;

	void push(ElementType value);

	ElementType pop(void);
};


/**
 * Thread-safe method for adding an element to the ringbuffer.
 * Blocks the thread if the ringbuffer is full.
 * The writing is critical only one thread can write at a time.
 */
template<typename ElementType>
void
RingBuffer<ElementType>::push(
	ElementType value
) {
	// if buffer full it blocks for one second
	if (!unwritten_elements.try_acquire_for(TIMEOUT)) {
		throw std::exception();
	};

	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};
	*write_pointer = value;

	// set write_pointer to next element
	write_pointer = (write_pointer == &buffer[RING_BUFFER_SIZE - 1]) ? buffer : write_pointer + 1;

	// increase counter of unread elements
	unread_elements.release();
}


/**
 * Thread-safe method for reading and removing an element from the queue.
 * Blocks the thread if queue is empty and waits till an element is in the queue.
 * The reading is critical only one thread can read at a time.
 */
template<typename ElementType>
ElementType
RingBuffer<ElementType>::pop(
	void
) {
	// if buffer empty it blocks for one second
	if (!unread_elements.try_acquire_for(TIMEOUT)) {
		throw std::exception();
	};

	// critical write section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};
	ElementType value = *read_pointer;

	// set read_pointer to next element
	read_pointer = (read_pointer == &buffer[RING_BUFFER_SIZE - 1]) ? buffer : read_pointer + 1;

	// increase counter of unread elements
	unwritten_elements.release();

	return value;
}

#endif /* QUEUE_HPP */
