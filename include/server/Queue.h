#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <queue>
#include <mutex>
#include <semaphore>

// NOTE(Lukas Karafiat): this is a bottleneck, but an ordered sequential bottleneck.  exactly what we want!
// multi producer, multi consumer queue with a central repository of relevant data
class Queue {
private:
	std::queue<int> buffer;

	std::mutex write_mutex;
	std::mutex read_mutex;

	std::counting_semaphore<32> unread_elements(0);

public:
	Queue(void) = default;
	~Queue(void) = default;

	void push(int);
	int pop(void);
};

#endif /* QUEUE_HPP */
