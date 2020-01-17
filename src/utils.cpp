//
// Created by Markus on 2019-09-28.
//

#include "utils.h"


namespace utils
{


    int64 toMicros(float val)
    {
        return (int64) (val * 1000000);
    }

    std::string loadFile(std::string filePath)
    {
        std::ifstream ifs(filePath);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        std::cerr << "hello" << std::endl;
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
}