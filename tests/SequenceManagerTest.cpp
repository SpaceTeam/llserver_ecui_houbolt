//
// Created by raffael on 29.03.25.
//

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>
#include  "SequenceManager.h"


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


// Demonstrate some basic assertions.
TEST(SequenceManager, Setup) {
    nlohmann::json config_json = {
            {"autoabort", false},
            {"WEBSERVER", {
                {"timer_sync_rate",10}
                }}
        };
    TestConfig config = TestConfig(config_json);

    EventManagerMock event_manager_mock = EventManagerMock();
    EXPECT_CALL(event_manager_mock, Init)
    .Times(AtLeast(1));


    EventManager::SetInstance(&event_manager_mock);


    SequenceManager manager = SequenceManager();
    manager.Init(config);


    EventManager* instance = EventManager::Instance();
    instance->Init(config);



    nlohmann::json sequence = {

    };


    EXPECT_FALSE(manager.IsSequenceRunning());
}
