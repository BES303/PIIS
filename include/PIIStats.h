#ifndef PIIStats_H
#define PIIStats_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>

//TODO: ######

struct ScanData
{
    std::string filePath;
    std::map<std::string, size_t> piiCounts;
    double durationSeconds;
    std::chrono::system_clock::time_point timestamp;
};

class PIIStats
{
public:
    PIIStats() = default;

    void addRecord(const std::string& filePath, const std::map<std::string, std::vector<std::string>>& results, double duration)
    {
        ScanData record;
        record.filePath = filePath;
        record.timestamp = std::chrono::system_clock::now();
        record.durationSeconds = duration;

        for (const auto& [type, matches] : results)
            record.piiCounts[type] = matches.size();

        _records.push_back(record);
    }

    size_t getTotalFiles() const
    {
        return _records.size();
    }

    std::map<std::string, size_t> getTotalPIICounts() const
    {
        std::map<std::string, size_t> total;

        for (const auto& record : _records)
            for (const auto& [type, count] : record.piiCounts)
                total[type] += count;

        return total;
    }

    double getAverageDuration() const
    {
        if (_records.empty()) return 0.0;
        double total = 0.0;

        for (const auto& record : _records)
            total += record.durationSeconds;

        return total / _records.size();
    }

    void saveToJson(const std::string& outputFile) const
    {
        nlohmann::json j;
        for (const auto& record : _records)
        {
            nlohmann::json r;
            r["filePath"] = record.filePath;
            r["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(record.timestamp.time_since_epoch()).count();
            r["durationSeconds"] = record.durationSeconds;
            r["piiCounts"] = record.piiCounts;
            j["records"].push_back(r);
        }
        j["totalFiles"] = getTotalFiles();
        j["totalPIICounts"] = getTotalPIICounts();
        j["averageDuration"] = getAverageDuration();

        std::ofstream out(outputFile);
        out << j.dump(4);
        out.close();
    }

    void printStatistics() const //TODO:move
    {
        std::cout << "\nСтатистика сканирования:\n";
        std::cout << "======================\n";
        std::cout << "Обработано файлов: " << getTotalFiles() << "\n";
        std::cout << "Среднее время обработки: " << getAverageDuration() << " сек\n";
        std::cout << "Найдено PII:\n";

        for (const auto& [type, count] : getTotalPIICounts())
            std::cout << "  " << type << ": " << count << "\n";

        std::cout << "======================\n";
    }

private:
    std::vector<ScanData> _records;
};

#endif // PIIStats_H
