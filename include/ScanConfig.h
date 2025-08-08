#ifndef SCANCONFIG_H
#define SCANCONFIG_H

#include <string>
#include <vector>
#include <map>

struct ScanConfig {
    std::filesystem::path filePath;    
    std::filesystem::path directoryPath;
    std::filesystem::path outputFile = "statistics.json";
    std::filesystem::path patternConfigFile;

    bool recursive = false;
    std::string strategy = "regex";

    std::map<std::string, std::vector<std::string>> patterns;
    std::map<std::string, std::vector<std::string>> keywords;
};

#endif // SCANCONFIG_H