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

