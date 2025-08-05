#ifndef PIIGENERALSTATS_H
#define PIIGENERALSTATS_H

#include <map>
#include <string>
#include <nlohmann/json.hpp>

class PIIGeneralStats
{
public:
    PIIGeneralStats() = default;

    void addRecord(const std::map<std::string, std::vector<std::string>>& results, double duration)
    {
        totalFiles++;
        totalDuration += duration;

        for (const auto& [type, matches] : results)
        {
            piiCounts[type] += matches.size();
            totalPII += matches.size();
        }
    }

    struct Stats
    {
        size_t totalFiles;
        size_t totalPII;
        double totalDuration;
        double avgDuration;
        std::map<std::string, size_t> piiCounts;
    };

    Stats getStats() const
    {
        return
        {
            totalFiles,
            totalPII,
            totalDuration,
            totalFiles > 0 ? totalDuration / totalFiles : 0.0,
            piiCounts
        };
    }

private:
    size_t totalFiles = 0;
    size_t totalPII = 0;
    double totalDuration = 0.0;
    std::map<std::string, size_t> piiCounts;
};

#endif // PIIGENERALSTATS_H