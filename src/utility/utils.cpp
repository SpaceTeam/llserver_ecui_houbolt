//
// Created by Markus on 2019-09-28.
//

#include <chrono>


#include "utility/utils.h"


namespace utils
{


    int64_t toMicros(double val)
    {
        return (int64_t) (val * 1000000);
    }

    std::string loadFile(std::string filePath)
    {
        std::ifstream ifs(filePath);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        ifs.close();
        return content;
    }

    void saveFile(std::string filePath, std::string content)
    {
        std::ofstream ostr(filePath);
        ostr << content;
        ostr.close();
    }

    bool keyExists(const nlohmann::json& j, const std::string& key)
    {
        return j.find(key) != j.end();
    }

    uint64_t getCurrentTimestamp()
    {
        const auto currTime = std::chrono::system_clock::now();
 
        return std::chrono::duration_cast<std::chrono::microseconds>(
                    currTime.time_since_epoch()).count();
    }
}