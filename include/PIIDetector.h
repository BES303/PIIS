#ifndef PIIDETECTOR_H
#define PIIDETECTOR_H

#include "PIIRecognizer.h"

class PIIDetector
{
public:
    explicit PIIDetector(std::unique_ptr<IStrategyScanner> strategy): _strategy(std::move(strategy)) {}

    struct DetectorResult
    {
        std::map<std::string, std::vector<std::string>> matches;
        double duration;
    };

    DetectorResult scan(const std::string& data)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto results = _strategy->scan(data);
        auto duration = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now() - start).count();

        return { std::move(results), duration };
    }

private:
    std::unique_ptr<IStrategyScanner> _strategy;
};

#endif // PIIDETECTOR_H