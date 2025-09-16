#ifndef SCANNER_H
#define SCANNER_H

#include <filesystem>
#include <vector>
#include <functional>
#include <memory>
#include <iostream>
#include "PIIDetector.h"
#include "PIIResultHandler.h"
#include "FileReaders.h"

struct DirWalker
{
    static std::vector<std::filesystem::path> getFiles(const std::filesystem::path& path, bool recursive,
        std::function<bool(const std::filesystem::path&)> fileFilter = {})
    {
        std::vector<std::filesystem::path> files;

        try
        {
            if (!std::filesystem::exists(path))
            {
                std::cerr << "Directory does not exist: " << path.string() << std::endl;
                return files;
            }

            auto processEntry = [&fileFilter, &files](const auto& entry)
            {
                if (entry.is_regular_file())
                {
                    const auto& filePath = entry.path();

                    if (!fileFilter || fileFilter(filePath))
                        files.push_back(filePath);
                }
            };

            if (recursive)
            {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(path,
                    std::filesystem::directory_options::skip_permission_denied |
                    std::filesystem::directory_options::follow_directory_symlink))
                {
                    processEntry(entry);
                }
            }

            else
            {
                for (const auto& entry : std::filesystem::directory_iterator(path,
                    std::filesystem::directory_options::skip_permission_denied |
                    std::filesystem::directory_options::follow_directory_symlink))
                {
                    processEntry(entry);
                }
            }
        }

        catch (const std::exception& e)
        {
            std::cerr << "Error walking directory " << path.string() << ": " << e.what() << std::endl;
        }

        return files;
    }
};

class PIIFileProcess
{
public:
    PIIFileProcess(PIIDetector& detector, PIIResultHandler& resultHandler, const FileReaderFactory& readerFactory):
          _detector(detector),
          _resultHandler(resultHandler),
          _reader(readerFactory) {}

    void processFile(const std::filesystem::path& filePath)
    {
        try
        {
            if (!_reader.isSupported(filePath))
            {
                std::cout << "Skipping unsupported file format: " << filePath.string() << std::endl;
                return;
            }

            auto reader = _reader.getReader(filePath);
            auto data = reader->readText(filePath);

            auto scanResult = _detector.scan(data);

            _resultHandler.processResult(filePath, scanResult);
        }

        catch (const std::exception& e)
        {
            std::cerr << "Error processing file " << filePath.string()
                      << ": " << e.what() << std::endl;
        }
    }

private:
    PIIDetector& _detector;
    PIIResultHandler& _resultHandler;
    const FileReaderFactory& _reader;
};

class PIIScanner
{
public:
    PIIScanner(PIIDetector& detector, PIIResultHandler& resultHandler,  const FileReaderFactory& readerFactory)
        : _detector(detector),
          _resultHandler(resultHandler),
          _reader(readerFactory),
          _fileProcess(detector, resultHandler, readerFactory) {}

    void scan(const std::filesystem::path& path, bool recursive = false)
    {
        std::vector<std::filesystem::path> files;

        if (std::filesystem::is_directory(path))
        {
            files = DirWalker::getFiles(path, recursive,
                [this](const auto& filePath)
                {
                    return _reader.isSupported(filePath);
                });
        }

        else if (std::filesystem::is_regular_file(path) && _reader.isSupported(path))
            files.push_back(path);

        else
        {
            std::cerr << "Error: Path is neither file nor directory: " << path.string() << std::endl;
            return;
        }

        if (files.empty())
        {
            std::cout << "No files to process." << std::endl;
            _resultHandler.finalize();
            return;
        }

        for (const auto& file: files)
            _fileProcess.processFile(file);

        _resultHandler.finalize();
    }

private:
    PIIDetector& _detector;
    PIIResultHandler& _resultHandler;
    const FileReaderFactory& _reader;

    PIIFileProcess _fileProcess; //
};

#endif // SCANNER_H
