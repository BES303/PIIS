#ifndef PIIDETECTOR_H
#define PIIDETECTOR_H

#include "PIIScanner.h"
#include "PIIResultHandler.h"

class PIIDetector
{
public:
    explicit PIIDetector(std::unique_ptr<IStrategyScanner> _strategy,
        const std::string& jsonOutput = ""): _strategy(std::move(_strategy))
    {
        _exporters.push_back(std::make_unique<ConsoleExporter>());

        if (!jsonOutput.empty())
            _exporters.push_back(std::make_unique<JsonExporter>(jsonOutput));
    }

    void addExporter(std::unique_ptr<IPIIResultExporter> exporter)
    {
        _exporters.push_back(std::move(exporter));
    }

    void scan(const std::string& data, const std::string& filePath)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto results = _strategy->scan(data);
        double duration = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now() - start).count();

        _generalStats.addRecord(results, duration);

        for (auto& exporter: _exporters)
            exporter->processFileResults(filePath, results, duration);
    }

    ~PIIDetector()
    {
        const auto generalStats = _generalStats.getStats();

        for (auto& exporter: _exporters)
            exporter->finalize(generalStats);
    }

private:
    PIIGeneralStats _generalStats;
    std::unique_ptr<IStrategyScanner> _strategy;
    std::vector<std::unique_ptr<IPIIResultExporter>> _exporters;
};

#endif // PIIDETECTOR_H