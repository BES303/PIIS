#ifndef CLI_H
#define CLI_H

#include <cxxopts.hpp>
#include "ScanConfig.h"

class CLI
{
public:
    CLI(int argc, char* argv[]);
    ScanConfig parse();
    void showHelp() const;
    bool isHelpRequested() const noexcept { return _result.count("help") > 0; }

private:
    cxxopts::Options _options;
    cxxopts::ParseResult _result;
};

#endif // CLI_H