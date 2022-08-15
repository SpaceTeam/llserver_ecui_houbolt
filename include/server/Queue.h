#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <queue>

// NOTE(Lukas Karafiat): this is a bottleneck, but an ordered sequential bottleneck.  exactly what we want!
// multi producer, multi consumer queue with a central repository of relevant data
class Queue {
private:
	std::queue<int> buffer;

	// TODO: lock(1,2) | blocks if another thread wants to write/read to the queue
	// TODO: semaphore | knows how many things are in the queue to read and blocks when there are none
public:
	Queue(void);
	~Queue(void);

	void push(int);
	int pop(void);
};

#endif /* QUEUE_HPP */
