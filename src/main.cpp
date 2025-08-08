#include <iostream>

#include "PIIConfigger.h"
#include "FileReader.h"
#include "PIIDetector.h"

void scanFile(const std::filesystem::path& filePath, PIIDetector& detector) //TODO
{
    if (!std::filesystem::exists(filePath))
    {
        std::cerr << "Error: File does not exist: " << filePath << std::endl;
        return;
    }

    auto reader = ReaderFactory::createReader(filePath);
    
    if (!reader)
        return;

    try
    {
        std::string data = reader->readText(filePath);
        detector.scan(data, filePath);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error reading file " << filePath << ": " << e.what() << std::endl;
    }
}

void scanDirectory(const std::filesystem::path& dirPath, bool recursive, PIIDetector& detector) //TODO
{
    try {
        if (!std::filesystem::exists(dirPath))
        {
            std::cerr << "Directory does not exist: " << dirPath << std::endl;
            return;
        }

        auto processFile = [&detector](const std::filesystem::path& filePath)
        {
            try
            {
                if (std::filesystem::is_regular_file(filePath))
                    scanFile(filePath.string(), detector);
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                std::cerr << "Filesystem error: " << filePath << " - " << e.what() << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error processing file: " << filePath << " - " << e.what() << std::endl;
            }
        };

        if (recursive)
        {
            for (const auto& entryFile : std::filesystem::recursive_directory_iterator(
                dirPath,
                std::filesystem::directory_options::skip_permission_denied |
                std::filesystem::directory_options::follow_directory_symlink))
            {
                processFile(entryFile.path());
            }
        }
        else
        {
            for (const auto& entryFile : std::filesystem::directory_iterator(
                dirPath,
                std::filesystem::directory_options::skip_permission_denied |
                std::filesystem::directory_options::follow_directory_symlink))
            {
                processFile(entryFile.path());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Filesystem error in directory scan: " << dirPath << " - " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    PIIConfigger configger (argc, argv);

    if (configger.isHelpRequested()) //TODO
    {
        configger.showHelp();
        return 0;
    }

    ScanConfig config = configger.getConfig();  //TODO: patterns

    if (config.filePath.empty() && config.directoryPath.empty())
    {
        std::cerr << "Error: Must specify a file or directory to scan" << std::endl;
        return 1;
    }

    auto piiStrategy = PIIStrategyHandler::createStrategy(config.strategy, config.patterns, config.keywords); //TODO: -/   /-
    
    PIIDetector detector(std::move(piiStrategy), config.outputFile);

    if (!config.filePath.empty())
        scanFile(config.filePath, detector);

    // TODO: 
    // 2 scan 1 file & dir

    if (!config.directoryPath.empty())
        scanDirectory(config.directoryPath, config.recursive, detector);

    return 0;
}
