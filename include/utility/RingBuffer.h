#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <mutex>
#include <semaphore>

template <typename T>
struct is_chrono_duration
{
	static constexpr bool value = false;
};

template <typename Rep, typename Period>
struct is_chrono_duration<std::chrono::duration<Rep, Period>>
{
	static constexpr bool value = true;
};

/**
 * Multi producer, multi consumer ring buffer as a central repository of relevant data.
 * Multiple threads can read and write without race-conditions.
 * This class is thread-safe (critical section).
 */
template<typename ElementType, int ring_buffer_size = 32, typename Duration = std::chrono::milliseconds, int timeout_length = 1000>
class RingBuffer {
private:
	static_assert(is_chrono_duration<Duration>::value, "duration must be a std::chrono::duration");

	ElementType buffer[ring_buffer_size];

	// Pointer for reading and writing position
	ElementType *read_pointer = buffer;
	ElementType *write_pointer = buffer;

	// Locks for the critical read and write section.
	std::mutex write_mutex;
	std::mutex read_mutex;

	// Semaphore for reading and writing elements.
	std::counting_semaphore<ring_buffer_size> unread_elements{0};
	std::counting_semaphore<ring_buffer_size> unwritten_elements{ring_buffer_size};

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
template<typename ElementType, int ring_buffer_size, typename Duration, int timeout_length>
void
RingBuffer<ElementType, ring_buffer_size, Duration, timeout_length>::push(
	ElementType value
) {
	// if buffer full it blocks for one second
	if (!unwritten_elements.try_acquire_for(Duration(timeout_length))) {
		throw std::exception();
	};

	// critical write section till end of function
	std::lock_guard<std::mutex> lock{write_mutex};
	*write_pointer = value;

	// set write_pointer to next element
	write_pointer = (write_pointer == &buffer[ring_buffer_size - 1]) ? buffer : write_pointer + 1;

	// increase counter of unread elements
	unread_elements.release();
}


/**
 * Thread-safe method for reading and removing an element from the queue.
 * Blocks the thread if queue is empty and waits till an element is in the queue.
 * The reading is critical only one thread can read at a time.
 */
template<typename ElementType, int ring_buffer_size, typename Duration, int timeout_length>
ElementType
RingBuffer<ElementType, ring_buffer_size, Duration, timeout_length>::pop(
	void
) {
	// if buffer empty it blocks for one second
	if (!unread_elements.try_acquire_for(Duration(timeout_length))) {
		throw std::exception();
	};

	// critical write section till end of function
	std::lock_guard<std::mutex> lock{read_mutex};
	ElementType value = *read_pointer;

	// set read_pointer to next element
	read_pointer = (read_pointer == &buffer[ring_buffer_size - 1]) ? buffer : read_pointer + 1;

	// increase counter of unread elements
	unwritten_elements.release();

	return value;
}

#endif /* QUEUE_HPP */
