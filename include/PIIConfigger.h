#ifndef PIICONFIGGER_H
#define PIICONFIGGER_H

#include <cxxopts.hpp>
#include <regex>
#include <string>
#include <map>
#include <vector>

struct ScanConfig
{
    std::string filePath;                              // Path to the file to scan
    std::string directoryPath;                         // Path to the directory to scan
    bool recursive = false;                            // Whether to scan directory recursively
    std::string strategy = "regex";                    // Scanning strategy (regex/keyword)
    std::string outputFile = "statistics.json";        // File to save statistics

    //////////////////////////////////////////////////////////////////////////////
    
    std::map<std::string, std::vector<std::regex>> patterns;    // Regex patterns for scanning
    std::map<std::string, std::vector<std::string>> keywords;   // Keywords for scanning
};

class PIIConfigger
{
public:
    PIIConfigger(int argc, char* argv[]);

    void showHelp() const;
    bool isHelpRequested() const { return _result.count("help") > 0; }
    ScanConfig getConfig() const;

private:
    cxxopts::Options _options;
    cxxopts::ParseResult _result;

    std::map<std::string, std::vector<std::regex>> _patterns;
    std::map<std::string, std::vector<std::string>> _keywords;

    void initPatternsAndKeywords();
};

#endif // PIICONFIGGER_H