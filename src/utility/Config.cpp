#include "utility/Config.h"

#include <utility/FileSystemAbstraction.h>

#include "utility/utils.h"
#include "common.h"

const std::string Config::CONFIG_FILE_NAME = "config.json";
const std::string Config::MAPPING_FILE_NAME = "mapping.json";


Config::Config(std::string configPath)
{
    configFilePath = configPath + "/" + CONFIG_FILE_NAME;
    mappingFilePath = configPath + "/" + MAPPING_FILE_NAME;

	try
	{
		data = nlohmann::json::parse(FileSystemAbstraction::Instance()->LoadFile(configFilePath));
	}
	catch(const std::exception& e)
	{
		std::cerr << "config file parsing failed; " << Config::configFilePath << std::endl;
        std::cerr << e.what() << std::endl;
		exit(1);
	}
}


std::string Config::getConfigFilePath()
{
    return configFilePath;
}


std::string Config::getMappingFilePath()
{
    return mappingFilePath;
}

nlohmann::json &Config::operator[](const std::string &path)
{
    auto pointer = nlohmann::json_pointer<nlohmann::json>(path);
    return Config::data[pointer];
}


void Config::print() {
    std::cerr << std::setw(4) << data << std::endl;
}

