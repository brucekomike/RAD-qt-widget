#include "configparser.h"
#include <iostream>

ConfigParser::ConfigParser(const std::string& filePath) : filePath(filePath) {}

void ConfigParser::parseConfig() {
    try {
        config = YAML::LoadFile(filePath);
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Error loading YAML file: " + std::string(e.what()));
    }
}

void ConfigParser::printConfig() {
    std::cout << config << std::endl;
}

void ConfigParser::showInfo(){
    auto info = config["info"];
    if (info) {
        for (auto it = info.begin(); it != info.end(); ++it) {
            std::cout << it->first.as<std::string>() << ": " << it->second.as<std::string>() << std::endl;
        }
    } else {
        std::cout << "No info section found in the configuration." << std::endl;
    }
}

PlatformConfig ConfigParser::getPlatformConfig(const std::string& platform, const std::string& architecture) const {
    PlatformConfig platformConfig;

    // Add common configurations if they exist
    if (config["common"]) {
        if (config["common"]["install"]) {
            for (const auto& cmd : config["common"]["install"]) {
                platformConfig.installCommands.push_back(cmd.as<std::string>());
            }
        }
        if (config["common"]["git"]) {
            for (const auto& repo : config["common"]["git"]) {
                platformConfig.gitRepos.push_back(repo.as<std::string>());
            }
        }
        if (config["common"]["files"]) {
            for (const auto& file : config["common"]["files"]) {
                FileCopy fileCopy;
                fileCopy.source = file["src"].as<std::string>();
                fileCopy.destination = file["dest"].as<std::string>();
                platformConfig.filesToCopy.push_back(fileCopy);
            }
        }
    }

    // Add platform-specific configurations if they exist
    if (config["platforms"][platform] && config["platforms"][platform][architecture]) {
        auto archNode = config["platforms"][platform][architecture];
        if (archNode["install"]) {
            for (const auto& cmd : archNode["install"]) {
                platformConfig.installCommands.push_back(cmd.as<std::string>());
            }
        }
        if (archNode["git"]) {
            for (const auto& repo : archNode["git"]) {
                platformConfig.gitRepos.push_back(repo.as<std::string>());
            }
        }
        if (archNode["files"]) {
            for (const auto& file : archNode["files"]) {
                FileCopy fileCopy;
                fileCopy.source = file["src"].as<std::string>();
                fileCopy.destination = file["dest"].as<std::string>();
                platformConfig.filesToCopy.push_back(fileCopy);
            }
        }
        if (archNode["checks"]) {
            for (const auto& check : archNode["checks"]) {
                Check checkStruct;
                checkStruct.command = check["command"].as<std::string>();
                for (const auto& failCmd : check["on_fail"]) {
                    checkStruct.onFailCommands.push_back(failCmd.as<std::string>());
                }
                platformConfig.checks.push_back(checkStruct);
            }
        }
    }

    return platformConfig;
}
