#ifndef GENERALCONFIG_H
#define GENERALCONFIG_H

#include <string>
#include <vector>
#include <map>
#include <filesystem>

struct GeneralConfig
{
    std::filesystem::path inputPath;
    std::filesystem::path outputJson;
    std::filesystem::path patternConfigFile;

    bool recursive = false;
    std::string strategy = "regex";

    std::map<std::string, std::vector<std::string>> patterns;
    std::map<std::string, std::vector<std::string>> keywords;

    static constexpr uint64_t MAX_TXT_SIZE = 200ULL * 1024 * 1024;
    static constexpr uint64_t MAX_PPTX_SIZE = 200ULL * 1024 * 1024;
    static constexpr uint64_t MAX_XML_SIZE = 200ULL * 1024 * 1024;
    static constexpr uint64_t MAX_DOCX_SIZE = 200ULL * 1024 * 1024;
    static constexpr uint64_t MAX_XLSX_SIZE = 200ULL * 1024 * 1024;
    static constexpr uint64_t MAX_PDF_SIZE = 200ULL * 1024 * 1024;
};

#endif // GENERALCONFIG_H