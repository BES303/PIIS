#ifndef CLP_H
#define CLP_H

#include <cxxopts.hpp>
#include "ScanConfig.h"

class clp
{
public:
    clp(int argc, char* argv[]);
    ScanConfig parse();
    void showHelp() const;
    bool isHelpRequested() const { return _result.count("help") > 0; }

private:
    cxxopts::Options _options;
    cxxopts::ParseResult _result;
};

#endif // CLP_H