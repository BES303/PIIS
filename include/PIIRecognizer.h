#ifndef PIISCANNER_H
#define PIISCANNER_H

#include <iostream>
#include <map>
#include <regex>
#include <re2/re2.h>

class IStrategyScanner
{
public:
    virtual ~IStrategyScanner() = default;
    virtual void onStart() {}
    virtual void onStop() {}
    virtual std::map<std::string, std::vector<std::string>> scan(const std::string& text) = 0;
};

class RegexStrategy: public IStrategyScanner
{
public:
    explicit RegexStrategy(const std::map<std::string, std::vector<std::string>>& patterns)
    {
        for (const auto& [type, regexList] : patterns)
        {
            for (const auto& pattern : regexList)
            {
                auto re = std::make_unique<re2::RE2>(pattern);

                if (!re->ok())
                    throw std::invalid_argument("Invalid regex pattern: " + pattern +
                                              " (error: " + re->error() + ")");

                _cmpPatterns[type].push_back(std::move(re));
            }

        }
    }

    std::map<std::string, std::vector<std::string>> scan(const std::string& text) override;

private:
    std::map<std::string, std::vector<std::unique_ptr<re2::RE2>>> _cmpPatterns;
};

class KeywordStrategy: public IStrategyScanner
{
public:
    explicit KeywordStrategy(const std::map<std::string, std::vector<std::string>>& keywords)
        : _keywords(keywords)
    {
        for (auto& [category, words] : _keywords)
        {
            for (auto& word : words)
            {
                std::string lowerWord = word;
                std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                _lowerKeywords[category].push_back(lowerWord);
            }
        }
    }

    std::map<std::string, std::vector<std::string>> scan(const std::string& text) override;

private:
    std::map<std::string, std::vector<std::string>> _keywords; // ???
    std::map<std::string, std::vector<std::string>> _lowerKeywords;
};

struct PIIStrategyHandler
{
    static std::unique_ptr<IStrategyScanner> createRegexStrategy(const std::map<std::string, std::vector<std::string>>& patterns)
    {
        return std::make_unique<RegexStrategy>(patterns);
    }

    static std::unique_ptr<IStrategyScanner> createKeywordStrategy(const std::map<std::string, std::vector<std::string>>& keywords)
    {
        return std::make_unique<KeywordStrategy>(keywords);
    }
};

#endif // PIISCANNER_H
