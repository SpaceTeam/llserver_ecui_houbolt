#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <mutex>
#include <semaphore>

template<typename T>
class RingBuffer {
  /**
   * This is a multi producer, multi consumer ringbuffer with a central repository of relevant data
   * That means multiple threads can read and write without race-conditions.
   * This class is thread-safe (critical section).
   */
 private:
  const static int RINGBUFFER_BUFF_SIZE = 128;

  T buffer[RINGBUFFER_BUFF_SIZE];

  // Pointer for reading and writing position
  T *read_pointer;
  T *write_pointer;

  // Locks for the critical read and write section.
  std::mutex write_mutex;
  std::mutex read_mutex;

  // Semaphore for reading and writing elements.
  std::counting_semaphore<RINGBUFFER_BUFF_SIZE> unread_elements{0};
  std::counting_semaphore<RINGBUFFER_BUFF_SIZE> unwritten_elements{RINGBUFFER_BUFF_SIZE};

 public:
  RingBuffer(void) = default;
  ~RingBuffer(void) = default;

  /**
   * Thread-safe methode for adding an element to the ringbuffer.
   * Blocks the thread if the ringbuffer is full.
   * The writing is critical only one thread can write at a time.
   */
  void push(T value);

  /**
   * Thread-safe methode for reading and removing an element from the queue.
   * Blocks the thread if queue is empty and waits till an element is in the queue.
   * The reading is critical only one thread can read at a time.
   */
  T pop(void);
};


template<typename T>
void RingBuffer<T>::push(T value) {
  // if buffer full it blocks
  unwritten_elements.acquire();

  // critical write section till end of function
  std::lock_guard<std::mutex> lock(write_mutex);
  *write_pointer = value;

  // set write_pointer to next element
  write_pointer = (write_pointer == &buffer[RINGBUFFER_BUFF_SIZE - 1]) ? buffer : write_pointer + 1;

  // increase counter of unread elements
  unread_elements.release();
}


template<typename T>
T RingBuffer<T>::pop(void) {
  // if buffer empty it blocks
  unread_elements.acquire();

  // critical write section till end of function
  std::lock_guard<std::mutex> lock(read_mutex);
  T value = *read_pointer;

  // set read_pointer to next element
  read_pointer = (read_pointer == &buffer[RINGBUFFER_BUFF_SIZE - 1]) ? buffer : read_pointer + 1;

  // increase counter of unread elements
  unwritten_elements.release();

  return value;
}

#endif /* QUEUE_HPP */
