#include "PIIResultHandler.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <chrono>
#include <iostream>

void ConsoleExporter::processFileResults(const std::string& filePath,
    const std::map<std::string, std::vector<std::string>>& results, double duration)
{
    std::cout << "\nScan completed: " << filePath << " (" << duration << "s)" << std::endl;

    for (const auto& [type, matches] : results)
    {
        std::cout << "  " << type << " (" << matches.size() << "):" << std::endl;

        for (const auto& match : matches)
            std::cout << "    - " << match << std::endl;
    }
}

void ConsoleExporter::finalize(const PIIGeneralStats::Stats& stats)
{
    std::cout << "\n===== FINAL STATISTICS =====\n";
    std::cout << "Files processed: " << stats.totalFiles << std::endl;
    std::cout << "Total PII found: " << stats.totalPII << std::endl;
    std::cout << "Total duration: " << stats.totalDuration << std::endl;
    std::cout << "Avg per file: " << stats.avgDuration << std::endl;
    std::cout << "PII counts by type:" << std::endl;

    for (const auto& [type, count] : stats.piiCounts)
        std::cout << "  " << type << ": " << count << std::endl;
}

JsonExporter::JsonExporter(const std::string& outputFile)
    : outputFile(outputFile)
{
    jsonData["records"] = nlohmann::json::array();
}

JsonExporter::~JsonExporter()
{
    finalizeAndWrite();
}

void JsonExporter::processFileResults(const std::string& filePath,
    const std::map<std::string, std::vector<std::string>>& results, double duration)
{
    nlohmann::json record;
    record["file"] = filePath;
    record["duration"] = std::chrono::duration<double>(duration).count();
    record["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();;

    nlohmann::json matches;

    for (const auto& [type, values] : results)
        matches[type] = values;

    record["matches"] = matches;
    jsonData["records"].push_back(record);
}

void JsonExporter::finalize(const PIIGeneralStats::Stats& stats)
{
    nlohmann::json statsJson;
    statsJson["totalFiles"] = stats.totalFiles;
    statsJson["totalPii"] = stats.totalPII;
    statsJson["totalDuration"] = stats.totalDuration;
    statsJson["avgDuration"] = std::chrono::duration<double>(stats.avgDuration).count();

    statsJson["piiCounts"] = stats.piiCounts;
    jsonData["statistics"] = statsJson;

    finalizeAndWrite();
}

void JsonExporter::finalizeAndWrite()
{
    std::ofstream file(outputFile, std::ios::trunc);

    if (!file)
    {
        std::cerr << "Error: Could not open output file " << outputFile;
        return;
    }
    file << jsonData.dump(4) << std::endl;
    file.close();
}