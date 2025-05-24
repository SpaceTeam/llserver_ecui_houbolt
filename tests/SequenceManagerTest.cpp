//
// Created by raffael on 29.03.25.
//

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>
#include  "SequenceManager.h"
#include "data/SequenceManagerTestData.h"
#include <utility/FileSystemAbstraction.h>
#include "LLInterface.h"

using ::testing::AtLeast;
class TestConfig : public Config {
    public:
    explicit TestConfig(nlohmann::json data) {
        this->data = std::move(data);
    }
};

class EventManagerMock : public EventManager {
public:

    MOCK_METHOD(void, Init, (Config &config), (override));
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, AddChannelTypes, ((std::map<std::string, std::string>& channelTypes)), (override));
    MOCK_METHOD(void, AddCommands, ((std::map<std::string, command_t> commands)), (override));
    MOCK_METHOD((std::map<std::string, command_t>), GetCommands, (), (override));
    MOCK_METHOD(void, OnStateChange, (const std::string& stateName, double oldValue, double newValue), (override));
    MOCK_METHOD(void, ExecuteRegexCommandOrState, (const std::string &regexKey, nlohmann::json &events, const std::string &stateName, double oldValue, double newValue, bool testOnly), (override));
    MOCK_METHOD(void, ExecuteCommandOrState, (const std::string &stateName, double oldValue, double newValue, bool useDefaultMapping, bool testOnly), (override));
    MOCK_METHOD(void, ExecuteCommand, (const std::string &commandName, std::vector<double> &params, bool testOnly), (override));
};

class FileSystemMock : public FileSystemAbstraction {
public:
    MOCK_METHOD(std::string, LoadFile, (const std::string &filePath),(override));
    MOCK_METHOD(void, SaveFile, (const std::string &filePath, const std::string &content),(override));
    MOCK_METHOD(void, CopyFile, (const std::string &src, const std::string &dst),(override));
    MOCK_METHOD(void, CreateDirectory, (const std::string &dirPath),(override));
};

class LLInterfaceMock : public LLInterface {
public:
    MOCK_METHOD(void, Init, (Config &config), (override));
    MOCK_METHOD(nlohmann::json, GetGUIMapping, (), (override));
    MOCK_METHOD(void, TransmitStates, (int64_t microTime, (std::map<std::string, std::tuple<double, uint64_t>> &states)), (override));
    MOCK_METHOD(void, StartStateTransmission, (Config &config), (override));
    MOCK_METHOD(void, StopStateTransmission, (), (override));
    MOCK_METHOD(nlohmann::json, GetAllStates, (), (override));
    MOCK_METHOD(nlohmann::json, GetAllStateLabels, (), (override));
    MOCK_METHOD(nlohmann::json, GetStates, (nlohmann::json &stateNames), (override));
    MOCK_METHOD(void, SetState, (std::string stateName, double value, uint64_t timestamp), (override));
    MOCK_METHOD(void, ExecuteCommand, (std::string &commandName, std::vector<double> &params, bool testOnly), (override));
    MOCK_METHOD((std::map<std::string, command_t>), GetCommands, (), (override));
    MOCK_METHOD((std::map<std::string, std::tuple<double, uint64_t>>), GetLatestSensorData, (), (override));
};

class SequenceManagerTest : public testing::Test {
protected:
    SequenceManagerTest() {


        file_system_mock = new testing::NiceMock<FileSystemMock>();
        FileSystemAbstraction::SetInstance(file_system_mock);

        nlohmann::json config_json = {
            {"autoabort", false},
            {"WEBSERVER", {
                    {"timer_sync_rate",10}
            }}
        };

        TestConfig config = TestConfig(config_json);

        event_manager_mock = new EventManagerMock();
        EventManager::SetInstance(event_manager_mock);
        event_manager_mock->Init(config);


        ll_interface_mock = new testing::NiceMock<LLInterfaceMock>();
        LLInterface::SetInstance(ll_interface_mock);

        sequenceManager = new SequenceManager();
        sequenceManager->Init(config);
        EXPECT_FALSE(sequenceManager->IsSequenceRunning());

    }
    ~SequenceManagerTest() override {
        while (sequenceManager->IsSequenceRunning()) {}

        delete event_manager_mock;
        delete sequenceManager;
        delete file_system_mock;
        delete ll_interface_mock;
    }

    EventManagerMock *event_manager_mock;
    SequenceManager *sequenceManager;
    FileSystemAbstraction *file_system_mock;
    LLInterfaceMock *ll_interface_mock;
};

TEST_F(SequenceManagerTest, StartIsExecutedOnlyOnce) {

    testing::Sequence executeCommandSequence;

    std::vector<double> value = {2};
    EXPECT_CALL(*event_manager_mock,ExecuteCommand("valve_1",value,false))
    .Times(1)
    .InSequence(executeCommandSequence);

    value = {0};
    EXPECT_CALL(*event_manager_mock,ExecuteCommand("valve_1",value,false))
    .Times(1)
    .InSequence(executeCommandSequence);

    nlohmann::json sequence = StartIsExecutedOnlyOnce_json;

    sequenceManager->StartSequence(sequence,nlohmann::json(),"");

}

TEST_F(SequenceManagerTest, LinearInterpolationIsCorrect) {
    using ::testing::_;
    using ::testing::Invoke;
    using ::testing::NiceMock;

    // We expect 100 calls, one every 0.01s for 1s
    const int expected_calls = 100;
    const double interval = 0.01;
    const double duration = 1.0;
    const double start_value = 0.0;
    const double end_value = 100.0;
    int call_count = 0;

    std::vector<std::pair<double, double>> observed;

    EXPECT_CALL(*event_manager_mock, ExecuteCommand("valve_1", _, false))
        .WillRepeatedly(Invoke([&](const std::string&, const std::vector<double>& params, bool) {
            double value = params[0];
            double t = call_count * interval;
            EXPECT_GE(value, start_value);
            EXPECT_LE(value, end_value);

            observed.emplace_back(t, value);
            call_count++;
        }));

    nlohmann::json sequence = LinearInterpolationTest1_json;
    sequenceManager->StartSequence(sequence, nlohmann::json(), "");

    // Wait for sequence to finish
    while (sequenceManager->IsSequenceRunning()) {}

    // We should have approximatly 100 calls.
    EXPECT_NEAR(observed.size(), expected_calls,1);
    // Check linearity: value should be start_value + (end_value - start_value) * (t/duration)
    for (int i = 0; i < expected_calls; ++i) {
        double t = observed[i].first;
        double value = observed[i].second;
        double expected = start_value + (end_value - start_value) * (t / duration);
        // Allow a small epsilon due to floating point and timing inaccuracies
        EXPECT_NEAR(value, expected, 5e-1) << "at t=" << t;
    }
}

TEST_F(SequenceManagerTest, AbortSequenceSetsValueAndStopsQuickly) {
    using ::testing::_;
    using ::testing::Invoke;
    using ::testing::Args;
    using ::testing::ElementsAre;
    using ::testing::Eq;

    // This matches the first argument as "valve_1" and the second as a vector with 10
    EXPECT_CALL(*event_manager_mock, ExecuteCommand("valve_1", ElementsAre(10), false))
        .Times(1);
    EXPECT_CALL(*event_manager_mock, ExecuteCommand("valve_1", ElementsAre(2), false));

    nlohmann::json sequence = StartIsExecutedOnlyOnce_json;
    nlohmann::json abort_sequence = SimpleAbortScenario_json;

    sequenceManager->StartSequence(sequence, abort_sequence, "");

    // Wait 0.5s, then abort
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sequenceManager->AbortSequence("test abort");

    // Wait for sequence to stop, but not forever
    auto abort_time = std::chrono::steady_clock::now();
    int waited_ms = 0;
    while (sequenceManager->IsSequenceRunning() && waited_ms < 1000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waited_ms += 10;
    }
    auto end = std::chrono::steady_clock::now();

    // Check that abort happened within a reasonable time (e.g., < 10ms after abort)
    auto abort_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - abort_time).count();
    EXPECT_LT(abort_duration, 10) << "Abort took too long to stop the sequence";
}
