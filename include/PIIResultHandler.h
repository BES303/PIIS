#ifndef PIIRESULTHANDLER_H
#define PIIRESULTHANDLER_H

#include "PIIGeneralStats.h"

class PIIResultHandler
{
public:
    explicit PIIResultHandler(std::vector<std::unique_ptr<IPIIResultExporter>> exporters,
        std::unique_ptr<PIIGeneralStats> stats = std::make_unique<PIIGeneralStats>())
        : _stats(std::move(stats)), _exporters(std::move(exporters))
    {
        if (_exporters.empty())
            throw std::invalid_argument("At least one exporter must be provided");
    }

    void processResult(const std::filesystem::path& filePath,
        const PIIDetector::DetectorResult& result)
    {
        _stats->addRecord(result.matches, result.duration);

        for (auto& exporter : _exporters)
            exporter->processFileResults(filePath, result.matches, result.duration);
    }

    void finalize()
    {
        auto stats = _stats->getStats();

        for (auto& exporter: _exporters)
            exporter->finalize(stats);
    }

private:
    std::unique_ptr<PIIGeneralStats> _stats;
    std::vector<std::unique_ptr<IPIIResultExporter>> _exporters;
};

#endif //PIIRESULTHANDLER_H
