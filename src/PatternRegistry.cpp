#include "PatternRegistry.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>



void PatternRegistry::registryProvider(const std::string& type, std::function<std::unique_ptr<IPatternProvider>()> creator) ////
{
    _providers[type] = std::move(creator);
}

void PatternRegistry::loadPatterns(const std::string& providerType)
{
    auto it = _providers.find(providerType);

    if (it == _providers.end())
    {
        std::cerr << "Provider error: " << providerType << std::endl;
        return;
    }

    auto provider = it->second();
    provider->provide(_patterns, _keywords);
}

void JsonProvider::provide(std::map<std::string, std::vector<std::string>>& patterns,
             std::map<std::string, std::vector<std::string>>& keywords) //
{
    if (_configFile.empty())
    {
        std::cerr << "Configuration file not specified" << std::endl;
        return;
    }

    std::ifstream file(_configFile);

    if (!file.is_open())
    {
        std::cerr << "Failed to open configuration file: " << _configFile << std::endl;
        return;
    }

    nlohmann::json json;

    try
    {
        file >> json;
    }
    catch (const nlohmann::json::exception& e)
    {
        throw std::runtime_error("Invalid JSON: " + std::string(e.what()));
    }

    if (json.contains("patterns"))
    {
        for (auto& [type, patternList] : json["patterns"].items())
            patterns[type] = patternList.get<std::vector<std::string>>();
    }

    if (json.contains("keywords"))
    {
        for (auto& [category, keywordList] : json["keywords"].items())
            keywords[category] = keywordList.get<std::vector<std::string>>();
    }
}

void DefaultProvider::provide(std::map<std::string, std::vector<std::string>>& patterns,
             std::map<std::string, std::vector<std::string>>& keywords)
{
    patterns =
    {
        {"email", {
            R"((?:^|\s)([A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,})(?:$|\s))",
            R"(<([A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,})>)",
            R"("([A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,}))"
        }},

        {"phone", {
            R"(\+(\d{11}|\d \d{3} \d{3}-\d{2}-\d{2}|\d \(\d{3}\)\s*\d{3}-\d{4})\b)",
            R"((\+7\s*\(\d{3}\)\s*\d{3}-\d{2}-\d{2}))"}},

        {"ip", {
            R"(\b((?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.(?:25[0-5]|2[0-4][0-9]|
            1[0-9]{2}|[1-9]?[0-9])\.(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9]))\b)"}},

        {"cardNumber", {
            R"((?:\b|\s|^)((\d[ -]?){15,16}\d)(?:\b|\s|$))",
            R"((?:\b|\s|^)(3[47]\d{2}[ -]?\d{6}[ -]?\d{5})(?:\b|\s|$))"
        }},

        {"passport", {
            R"((?:^|\s)(\d{4}[\s-]?\d{6})(?:$|\s))",
            R"(\b\d{2}\s?\d{2}\s?\d{6}\b)",
            R"(\b[A-Za-z]{2}\s?\d{7}\b)"
        }},

        {"url", {
            R"((?:^|\s)(https?://[A-Za-z0-9\-._~:/?#[\]@!$&'()*+,;=%]+)(?:$|\s))",
            R"((?:^|\s)(www\.[A-Za-z0-9\-._~:/?#[\]@!$&'()*+,;=%]+)(?:$|\s))",
            R"((?:^|\s)([A-Za-z0-9\-.]+\.[A-Za-z]{2,}(?:/[A-Za-z0-9\-._~:/?#[\]@!$&'()*+,;=%]*)?)(?:$|\s))"
        }}
    };

    keywords =
    {
        {"sensitive", {"confidential", "password", "token", "financial", "secret", "restricted"}},
        {"personal",  {"name", "passport", "personal", "identification", "id", "user"}}
    };
}