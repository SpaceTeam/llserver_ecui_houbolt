//
// Created by Christofer Held on 13.08.22.
//
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <thread>
#include <optional>

#include "WebSocket.h"
#include "WebSocketClient.h"
#include "control_flag.h"

volatile sig_atomic_t finished = false;
// NOTE(Lukas Karafiat): these tests should run sequentially as finished is a
//     global variable that every running component has access to and will be
//     shut down if set to true
TEST(WritingAndReadingFromWebSocket, WriteFullReadAll_shouldBeInOrder) {
	const int size = 32;

	std::shared_ptr<RingBuffer<std::string, size>> server_request_queue(new RingBuffer<std::string, size>());
	std::shared_ptr<RingBuffer<std::string, size>> server_response_queue(new RingBuffer<std::string, size>());

	std::shared_ptr<RingBuffer<std::string, size>> client_request_queue(new RingBuffer<std::string, size>());
	std::shared_ptr<RingBuffer<std::string, size>> client_response_queue(new RingBuffer<std::string, size>());

	std::shared_ptr<WebSocket<1024>> server(new WebSocket("8080", server_request_queue, server_response_queue));
	std::shared_ptr<WebSocketClient> client(new WebSocketClient("localhost", "8080", client_request_queue, client_response_queue));

	std::jthread client_thread(&WebSocketClient::run, client);
	std::jthread server_thread(&WebSocket<1024>::run, server);

	std::optional<std::string> message_buffer = std::nullopt;
	
	while(client_response_queue->push("marco") == false);
	while ((message_buffer = server_request_queue->pop()) == std::nullopt);

	ASSERT_EQ(message_buffer.value(), "marco");

	while(server_response_queue->push("polo") == false);
	while ((message_buffer = client_request_queue->pop()) == std::nullopt);

	ASSERT_EQ(message_buffer.value(), "polo");

	finished = true;
}

