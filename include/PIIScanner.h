#ifndef PIISCANNER_H
#define PIISCANNER_H

#include <iostream>
#include <map>
#include <regex>
#include <atomic>
#include "PIIStats.h"

class IStrategyScanner
{
public:
    virtual ~IStrategyScanner() = default;
    virtual void onStart() {}
    virtual void onStop() {}
    virtual std::map<std::string, std::vector<std::string>> scan(const std::string& text) = 0;
};

class RegexStrategy : public IStrategyScanner
{
public:
    explicit RegexStrategy(const std::map<std::string, std::vector<std::regex>>& patterns)
        : _patterns(patterns) {}

    std::map<std::string, std::vector<std::string>> scan(const std::string& text) override;

private:
    std::map<std::string, std::vector<std::regex>> _patterns;
};

class KeywordStrategy : public IStrategyScanner
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

class ScannerController
{
public:
    enum class State { Running, Paused, Stopping, Stopped };

    ScannerController() : _state(State::Stopped), _piiStats(std::make_unique<PIIStats>()) {}

    void setStrategy(std::unique_ptr<IStrategyScanner> strategy)
    {
        _strategy = std::move(strategy);
    }

    void start(const std::string& data, const std::string& filePath)
    {
        if (!_strategy)
            throw std::runtime_error("Scanning strategy not set");

        if (_state == State::Running)
        {
            std::cout << "Scanning already in progress" << std::endl;
            return;
        }

        _state = State::Running;
        _strategy->onStart();

        auto startTime = std::chrono::high_resolution_clock::now();

        try
        {
            auto results = _strategy->scan(data);
            auto endTime = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double>(endTime - startTime).count();
            _piiStats->addRecord(filePath, results, duration);
            processResults(results);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Scanning error: " << e.what() << std::endl;
        }

        _strategy->onStop();
        _state = State::Stopped;
        _piiStats->printStatistics();
    }

    void saveStatistics(const std::string& outputFile) const
    {
        _piiStats->saveToJson(outputFile);
    }

private:
    
    void processResults(const std::map<std::string, std::vector<std::string>>& results)
    {
        std::cout << "\nScanning results:\n"; // TODO: move res
        std::cout << "=================\n";

        for (const auto& [type, matches] : results)
        {
            std::cout << type << " (" << matches.size() << " matches):\n";

            for (const auto& match : matches)
                std::cout << "  - " << match << "\n";

            std::cout << std::endl;
        }
    }

    std::unique_ptr<IStrategyScanner> _strategy;
    std::atomic<State> _state;
    std::unique_ptr<PIIStats> _piiStats;
};

#endif // PIISCANNER_H
