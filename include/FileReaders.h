#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <libzippp/libzippp.h>
#include "pugixml.hpp"

#include <vector>
#include <map>
#include <functional>
#include <memory>

// TODO: OCREngine & image reader
class ReaderBase
{
public:
    virtual std::string readText(const std::filesystem::path& filePath) = 0;
    virtual ~ReaderBase() {}
};

class TxtReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override;
};

class PdfReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override;
};

class XmlReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override;

private:
    void extractDataFromXml(const std::string& xmlData, std::string& data);
    void processXmlNode(const pugi::xml_node& node, std::string& data);
};

class PptxReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override;

private:
    void extractPptxData(libzippp::ZipArchive& zip, std::string& content);
    void extractSlideData(const std::string& xmlData, std::string& content);
    void processSlideNode(const pugi::xml_node& node, std::string& content);
};

class DocxReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override;
private:
    void extractDocxData(libzippp::ZipArchive& zip, std::string& resultData);
    void extractXmlData(const std::string& xmlData, std::string& resultData);
    void processXmlNode(const pugi::xml_node& node, std::string& resultData);
};

class XlsxReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override;
};

class FileReaderFactory
{
public:
    void registerReader(const std::string& extension, std::function<std::unique_ptr<ReaderBase>()> readerCreator)
    {
        const auto normalizedExt = normalizeExtension(extension);
        _readers[normalizedExt] = std::move(readerCreator);
    }

    template<typename T>
    void registerReader(const std::string& extension)
    {
        registerReader(extension, []() { return std::make_unique<T>(); });
    }

    template<typename T>
    void registerReader(const std::vector<std::string>& extensions)
    {
        for (const auto& extension: extensions)
            registerReader(extension, []() { return std::make_unique<T>(); });
    }

    std::unique_ptr<ReaderBase> getReader(const std::filesystem::path& filePath) const
    {
        const auto normalizedExt = normalizeExtension(filePath.extension().string());

        if (auto it = _readers.find(normalizedExt); it != _readers.end())
            return it->second();

        throw std::runtime_error("Unsupported file format: " + filePath.extension().string());
    }

    bool isSupported(const std::filesystem::path& filePath) const
    {
        const auto normalizedExt = normalizeExtension(filePath.extension().string());
        return _readers.contains(normalizedExt);
    }

    std::vector<std::string> getSupportedExtensions() const
    {
        std::vector<std::string> extensions;
        extensions.reserve(_readers.size());

        for (const auto& [ext, _] : _readers)
            extensions.push_back(ext);

        return extensions;
    }

    bool isEmpty() const
    {
        return _readers.empty();
    }

    size_t size() const
    {
        return _readers.size();
    }

    static std::string normalizeExtension(std::string ext)
    {
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return ext;
    }

private:
    std::map<std::string, std::function<std::unique_ptr<ReaderBase>()>> _readers;
};

#endif // FILEREADER_H
