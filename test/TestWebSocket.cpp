//
// Created by Christofer Held on 13.08.22.
//
#include <gtest/gtest.h>

#include <memory>
#include <thread>

#include "WebSocket.h"
#include "WebSocketClient.h"
#include "control_flag.h"

volatile sig_atomic_t finished = false;

void
run_server(
	void
) {
	return;
}

TEST(WritingAndReadingFromWebSocket, WriteFullReadAll_shouldBeInOrder) {
	const int size = 32;

	std::shared_ptr<RingBuffer<std::string, size>> server_request_queue(new RingBuffer<std::string, size>());
	std::shared_ptr<RingBuffer<std::string, size>> server_response_queue(new RingBuffer<std::string, size>());

	std::shared_ptr<RingBuffer<std::string, size>> client_request_queue(new RingBuffer<std::string, size>());
	std::shared_ptr<RingBuffer<std::string, size>> client_response_queue(new RingBuffer<std::string, size>());

	std::shared_ptr<WebSocket<1024>> server(new WebSocket("8080", server_request_queue, server_request_queue));

//	std::jthread test(&WebSocket::run, )

	std::shared_ptr<WebSocketClient> client(new WebSocketClient("localhost", "8080", client_request_queue, client_request_queue));

	std::jthread client_thread(&WebSocketClient::run, client);
	std::jthread server_thread(&WebSocket<1024>::run, server);

//	server->run();
//	client->run();

	client_request_queue->push("hello");
	ASSERT_EQ(server_request_queue->pop(), "hello");

	finished = true;

	ASSERT_EQ(1, 1);
}

