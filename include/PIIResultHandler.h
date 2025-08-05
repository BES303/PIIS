#ifndef PIIRESULTHANDLER_H
#define PIIRESULTHANDLER_H

#include "PIIGeneralStats.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

class IPIIResultExporter
{
public:
    virtual ~IPIIResultExporter() = default;
    virtual void processFileResults(const std::string& filePath,
        const std::map<std::string, std::vector<std::string>>& results, double duration) = 0;

    virtual void finalize(const PIIGeneralStats::Stats& stats) = 0;
};

class ConsoleExporter : public IPIIResultExporter
{
public:
    void processFileResults(const std::string& filePath,
        const std::map<std::string, std::vector<std::string>>& results, double duration) override;

    void finalize(const PIIGeneralStats::Stats& stats) override;
};

class JsonExporter : public IPIIResultExporter
{
public:
    explicit JsonExporter(const std::string& outputFile);
    ~JsonExporter();

    void processFileResults(const std::string& filePath,
        const std::map<std::string, std::vector<std::string>>& results, double duration) override;

    void finalize(const PIIGeneralStats::Stats& stats) override;
private:
    std::string outputFile;
    nlohmann::json jsonData;

    void finalizeAndWrite();
};

#endif // PIIRESULTHANDLER_H