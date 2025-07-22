#include <iostream>
#include <filesystem>
#include <memory>
#include "PIIConfigger.h"
#include "FileReader.h"
#include "PIIScanner.h"


void scanFile(const std::string& filePath, ScannerController& controller)
{

    if (!std::filesystem::exists(filePath))
    {
        std::cerr << "Error: File does not exist: " << filePath << std::endl;
        return;
    }

    auto reader = FileReaderFactory::createReader(filePath);
    
    if (!reader)
        return;

    try
    {
        std::string data = reader->readText(filePath);
        controller.start(data, filePath);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error reading file " << filePath << ": " << e.what() << std::endl;
    }
}

void scanDirectory(const std::string& dirPath, bool recursive, ScannerController& controller) //TODO: async
{
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath))
        return;

    if (recursive)
    {
        for (const auto& file : std::filesystem::recursive_directory_iterator(
                dirPath, std::filesystem::directory_options::skip_permission_denied))
            if (file.is_regular_file())
                scanFile(file.path().string(), controller);
    }
    else
    {
        for (const auto& file : std::filesystem::directory_iterator(
                dirPath, std::filesystem::directory_options::skip_permission_denied))
            if (file.is_regular_file())
                scanFile(file.path().string(), controller);
    }
}

int main(int argc, char* argv[])
{
    PIIConfigger configger (argc, argv);

    if (configger.isHelpRequested())
    {
        configger.showHelp();
        return 0;
    }

    ScanConfig config = configger.getConfig();

    if (config.filePath.empty() && config.directoryPath.empty())
    {
        std::cerr << "Error: Must specify a file or directory to scan\n";
        configger.showHelp();
        return 1;
    }

    ScannerController scannerController;

    if (config.strategy == "regex")
        scannerController.setStrategy(std::make_unique<RegexStrategy>(config.patterns));
    else if (config.strategy == "keyword")
        scannerController.setStrategy(std::make_unique<KeywordStrategy>(config.keywords));
    else
    {
        std::cerr << "Error: Unknown scanning strategy. Valid values: regex, keyword, combined\n";
        return 1;
    }

    if (!config.filePath.empty())
        scanFile(config.filePath, scannerController);

    // TODO: 
    // 2 scan 1 file & dir

    if (!config.directoryPath.empty())
        scanDirectory(config.directoryPath, config.recursive, scannerController);

    scannerController.saveStatistics(config.outputFile);

    return 0;
}
