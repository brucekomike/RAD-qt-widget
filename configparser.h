#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

struct FileCopy {
    std::string source;
    std::string destination;
};

struct Check {
    std::string command;
    std::vector<std::string> onFailCommands;
};

struct PlatformConfig {
    std::vector<std::string> installCommands;
    std::vector<std::string> gitRepos;
    std::vector<FileCopy> filesToCopy;
    std::vector<Check> checks;
};

class ConfigParser {
public:
    ConfigParser(const std::string& filePath);
    void parseConfig();
    void printConfig();
    void showInfo();
    PlatformConfig getPlatformConfig(const std::string& platform, const std::string& architecture) const;

private:
    std::string filePath;
    YAML::Node config;

};

#endif // CONFIGPARSER_H
