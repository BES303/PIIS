#include "PIIConfigger.h"
#include <iostream>

PIIConfigger::PIIConfigger(int argc, char* argv[])
    : _options("PIIScanner", "Program for searching personal data in files")
{

    _options.add_options()
        ("f,file", "Scan a specific file",
            cxxopts::value<std::string>())
        ("d,directory", "Scan a directory",
            cxxopts::value<std::string>())
        ("r,recursive", "Recursive scanning (...)",
            cxxopts::value<bool>()->default_value("false"))
        ("s,strategy", "Scanning strategy (regex/keyword)",
            cxxopts::value<std::string>()->default_value("regex"))
        ("o,output", "File to save statistics",
            cxxopts::value<std::string>()->default_value("statistics.json"))
        ("h,help", "Show help");

    try
    {
        _result = _options.parse(argc, argv);
    } 
    catch (const cxxopts::exceptions::exception& e)
    {
        std::cerr << "Error parsing parameters: " << e.what() << std::endl;
        showHelp();
        throw;
    }

    initPatternsAndKeywords();
}

void PIIConfigger::showHelp() const
{
    std::cout << _options.help() << std::endl;
}

void PIIConfigger::initPatternsAndKeywords()
{
    _patterns = //TODO: have regex segfault
    {
        {"email", {std::regex("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}")}},
        {"phone", {
                   std::regex("\\+[1-9][0-9]{0,2}\\d{7,12}"),
                   std::regex("\\+[1-9][0-9]{0,2}(?:[ .-]\\d{2,4}){2,4}"),
                   }},
        {"ip", {std::regex("(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)){3}")}},
        {"card_number", {std::regex("\\d{4}[- ]?\\d{4}[- ]?\\d{4}[- ]?\\d{4}")}},
        {"passport", {std::regex("[A-Z]{1,2}[0-9]{6,8}")}},
        {"url", {std::regex("https?://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/[a-zA-Z0-9._~%-]*)*")}},
        {"date", {std::regex("(0[1-9]|[12][0-9]|3[01])[-/.](0[1-9]|1[0-2])[-/.](19|20)[0-9]{2}")}}
    };

    _keywords =
    {
        {"sensitive", {"confidential", "password", "financial", "secret", "restricted"}},
        {"personal", {"name", "passport", "personal", "identification", "id"}}
    };
}

ScanConfig PIIConfigger::getConfig() const
{
    ScanConfig config;

    if (_result.count("file"))
        config.filePath = _result["file"].as<std::string>();

    if (_result.count("directory"))
        config.directoryPath = _result["directory"].as<std::string>();

    config.recursive = _result["recursive"].as<bool>();
    config.strategy = _result["strategy"].as<std::string>();
    config.outputFile = _result["output"].as<std::string>();
    config.patterns = _patterns;
    config.keywords = _keywords;

    return config;
}
