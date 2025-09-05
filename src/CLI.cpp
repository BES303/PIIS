#include "CLI.h"
#include <iostream>
#include <stdexcept>

CLI::CLI(int argc, char* argv[])
    : _options("PIIScanner", "Tool for detecting personally identifiable information in files")
{
    _options.add_options()
        ("f,file", "Scan specific file", cxxopts::value<std::string>())
        ("d,directory", "Scan directory", cxxopts::value<std::string>())
        ("r,recursive", "Recursive scanning", cxxopts::value<bool>()->default_value("false"))
        ("s,strategy", "Scanning strategy (regex/keyword/combined)", cxxopts::value<std::string>()->default_value("regex"))
        ("j,json", "Enable JSON export (saves to statistics.json)", cxxopts::value<std::string>()->implicit_value("statistics.json"))
        ("p,pattern-config", "Pattern configuration file", cxxopts::value<std::string>()->default_value(""))
        ("h,help", "Show help message");

    try
    {
        _result = _options.parse(argc, argv);
    }
    catch (const cxxopts::exceptions::exception& e)
    {
        std::cerr << "Parameter parsing error: " << e.what() << std::endl;
        showHelp();
        throw;
    }
}

void CLI::showHelp() const
{
    std::cout << _options.help() << std::endl;
}

GeneralConfig CLI::parse()
{
    GeneralConfig config;

    if (_result.count("file") && _result.count("directory"))
        throw std::runtime_error("Error: Cannot specify both file and directory simultaneously");


    if (_result.count("file"))
        config.inputPath = _result["file"].as<std::string>();

    else if (_result.count("directory"))
        config.inputPath = _result["directory"].as<std::string>();

    config.recursive = _result["recursive"].as<bool>();
    config.strategy = _result["strategy"].as<std::string>();

    if (_result.count("json"))
        config.outputJson = _result["json"].as<std::string>();

    config.patternConfigFile = _result["pattern-config"].as<std::string>();
    return config;
}