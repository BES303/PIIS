#ifndef SCANCONFIG_H
#define SCANCONFIG_H

#include <string>
#include <vector>
#include <map>

struct ScanConfig
{
    std::string filePath;
    std::string directoryPath;
    bool recursive = false;
    std::string strategy = "regex";
    std::string outputFile = "statistics.json";
    std::string patternConfigFile;
    std::map<std::string, std::vector<std::string>> patterns;
    std::map<std::string, std::vector<std::string>> keywords;
};

#endif // SCANCONFIG_H