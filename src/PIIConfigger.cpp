#include "PIIConfigger.h"
#include "PatternRegistry.h"


PIIConfigger::PIIConfigger(int argc, char* argv[]): _parser(std::make_unique<CLI>(argc, argv)),
    _patternRegistry(std::make_unique<PatternRegistry>()) {}

bool PIIConfigger::isHelpRequested() const
{
    return _parser->isHelpRequested();
}

void PIIConfigger::showHelp() const
{
    return _parser->showHelp();
}

GeneralConfig PIIConfigger::getConfig() const
{
    GeneralConfig config = _parser->parse();

    if (!config.patternConfigFile.empty())
    {
        auto configFile = config.patternConfigFile.string();
        _patternRegistry->registryProvider("json", [configFile]() { return std::make_unique<JsonProvider>(configFile); });
        _patternRegistry->loadPatterns("json");
    }
    else
    {
        _patternRegistry->registryProvider("default", []() { return std::make_unique<DefaultProvider>(); });
        _patternRegistry->loadPatterns("default");
    }

    config.patterns = _patternRegistry->getPatterns();
    config.keywords = _patternRegistry->getKeywords();
    return config;
}