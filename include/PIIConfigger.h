#ifndef PIICONFIGGER_H
#define PIICONFIGGER_H

#include "clp.h"
#include "ScanConfig.h"
#include "PatternRegistry.h"

class PIIConfigger
{
public:
    PIIConfigger(int argc, char* argv[]);
    ScanConfig getConfig() const;
    void showHelp() const;
    bool isHelpRequested() const;

private:
    std::unique_ptr<clp> _parser;
    std::unique_ptr<PatternRegistry> _patternRegistry;
};

#endif // PIICONFIGGER_H