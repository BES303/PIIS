#include "PIIResultExporter.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>

void ConsoleExporter::processFileResults(const std::filesystem::path& filePath,
    const std::map<std::string, std::vector<std::string>>& results, double duration)
{
    std::cout << "\n================= Scan Report =================" << std::endl;
    std::cout << " File     : " << filePath << std::endl;
    std::cout << " Duration : " << std::fixed << std::setprecision(2) << duration << std::endl;

    if (results.empty())
        std::cout << " Status   : No PII found." << std::endl;

    else
    {
        std::cout << " Status   : PII found!" << std::endl;
        std::cout << " Matches  :" << std::endl;

        for (const auto& [type, matches] : results)
        {
            std::cout << "  - " << type << " (" << matches.size() << "):" << std::endl;

            for (const auto& match : matches)
                std::cout << "       " << match << std::endl;

        }
    }
    std::cout << "==============================================" << std::endl;
}


void ConsoleExporter::finalize(const PIIGeneralStats::Stats& stats)
{
    std::cout << "\n============== FINAL SCAN SUMMARY ==============" << std::endl;
    std::cout << " Total files scanned : " << stats.totalFiles << std::endl;
    std::cout << " Total PII found     : " << stats.totalPII << std::endl;
    std::cout << " Total duration      : " << std::fixed << std::setprecision(2) << stats.totalDuration << "s" << std::endl;
    std::cout << " Avg duration/file   : " << std::fixed << std::setprecision(2) << stats.avgDuration << "s" << std::endl;

    if (!stats.piiCounts.empty())
    {
        std::cout << "\n PII found by type:" << std::endl;

        for (const auto& [type, count] : stats.piiCounts)
            std::cout << "  - " << type << ": " << count << std::endl;

    }
    else 
        std::cout << " No PII types detected." << std::endl;

    std::cout << "=================================================" << std::endl;
}


JsonExporter::JsonExporter(const std::filesystem::path& outputFile): outputFile(outputFile)
{
    jsonData["records"] = nlohmann::json::array();
}

JsonExporter::~JsonExporter() = default;

void JsonExporter::processFileResults(const std::filesystem::path& filePath,
    const std::map<std::string, std::vector<std::string>>& results, double duration)
{
    nlohmann::json record;
    record["file"] = filePath;
    record["duration"] = std::chrono::duration<double>(duration).count();
    record["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();

    nlohmann::json matches;

    for (const auto& [type, values]: results)
        matches[type] = values;

    record["matches"] = matches;
    jsonData["records"].push_back(record);
}

void JsonExporter::finalize(const PIIGeneralStats::Stats& stats)
{
    nlohmann::json statsJson;
    statsJson["total_files"] = stats.totalFiles;
    statsJson["total_pii"] = stats.totalPII;
    statsJson["total_duration"] = stats.totalDuration;
    statsJson["avg_duration"] = stats.avgDuration;

    statsJson["pii_counts"] = stats.piiCounts;
    jsonData["statistics"] = statsJson;

    std::ofstream file(outputFile, std::ios::trunc);

    if (!file)
        throw std::runtime_error("Could not open output file: " + outputFile.string());

    file << jsonData.dump(4) << std::endl;
}