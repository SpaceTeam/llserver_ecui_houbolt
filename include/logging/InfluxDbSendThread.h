//
// Created by raffael on 29.09.25.
//

#ifndef LLSERVER_ECUI_HOUBOLT_INFLUXDBSENDTHREAD_H
#define LLSERVER_ECUI_HOUBOLT_INFLUXDBSENDTHREAD_H
#include <atomic>
#include <memory>

#include "influxDb.h"
#include "blockingconcurrentqueue.h"

class InfluxDbWriter; // forward declaration

class InfluxDbSendThread {
public:
    explicit InfluxDbSendThread(std::shared_ptr<influxDbContext> context,
                       std::shared_ptr<moodycamel::BlockingConcurrentQueue<std::string>> queue,
                       InfluxDbWriter& influx_db_writer);

    ~InfluxDbSendThread();

    void stop();
    void join(); // new: join underlying thread
    void pushBuffer(std::string buffer);
    [[nodiscard]] bool isRunning() const;


private:
    std::atomic_bool running{false};
    std::atomic_int32_t messagesInQueue{0};
    std::mutex enqueueMutex;
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<std::string>> queue;
    std::shared_ptr<influxDbContext> context;
    std::thread thread;
    InfluxDbWriter& writer; // reference to owning writer

    void messageSendingLoop();
};
#endif //LLSERVER_ECUI_HOUBOLT_INFLUXDBSENDTHREAD_H