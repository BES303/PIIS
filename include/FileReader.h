#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
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
    std::string readText(const std::filesystem::path& filePath) override
    {
        std::ifstream file(filePath, std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + filePath.string());

        std::string fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return fileData;
    }
};

class PdfReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override
    {
        try
        {
            auto u8Path = filePath.u8string();
            std::string utf8Path(u8Path.begin(), u8Path.end());
            auto pdf = poppler::document::load_from_file(utf8Path);

            if (!pdf)
                throw std::runtime_error("Failed to load PDF: " + filePath.string());

            if (pdf->is_locked())
                throw std::runtime_error("Encrypted PDF not supported: " + filePath.string());

            std::string text;
            const auto pages = pdf->pages();

            for (auto page = 0; page < pages; ++page)
            {
                std::unique_ptr<poppler::page> pageData(pdf->create_page(page));
                if (pageData)
                {
                    auto byte_array = pageData->text().to_utf8();
                    text += std::string(byte_array.begin(), byte_array.end());
                }
            }

            return text;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("PDF processing error: " + std::string(e.what()));
        }
    }
};

class DocReader: public ReaderBase
{
public:
    std::string readText(const std::filesystem::path& filePath) override
    {
        throw std::runtime_error("DOC not supported");
    }
};

class ReaderRegistry
{
public:

    void registerReader(const std::string& extension, std::function<std::unique_ptr<ReaderBase>()> readerCreator)
    {
        const auto normalizedExt = normalizeExtension(extension);
        _readers[normalizedExt] = std::move(readerCreator);
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

        for (const auto& extension: _readers)
            extensions.push_back(extension.first);

        return extensions;
    }

    void registerReaderForExtensions(const std::vector<std::string>& extensions,
        std::function<std::unique_ptr<ReaderBase>()> readerCreator)
    {
        for (const auto& extension : extensions)
            registerReader(extension, readerCreator);
    }

    bool isEmpty() const
    {
        return _readers.empty();
    }

    size_t size() const
    {
        return _readers.size();
    }

    static std::string normalizeExtension(std::string ext) // .
    {
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return ext;
    }

private:
    std::map<std::string, std::function<std::unique_ptr<ReaderBase>()>> _readers;
};

class ReaderFactory
{
public:
    ReaderFactory() : _registry(std::make_unique<ReaderRegistry>())
    {}

    explicit ReaderFactory(std::unique_ptr<ReaderRegistry> registry) : _registry(std::move(registry))
    {
        if (!_registry)
            throw std::invalid_argument("Registry cannot be null");
    }

    template<typename T>
    void registerReader(const std::string& extension)
    {
        _registry->registerReader(extension, []() { return std::make_unique<T>(); });
    }

    template<typename T>
    void registerReaderForExtensions(const std::vector<std::string>& extensions)
    {
        _registry->registerReaderForExtensions(extensions, []() { return std::make_unique<T>(); });
    }

    std::unique_ptr<ReaderBase> getReader(const std::filesystem::path& filePath) const
    {
        return _registry->getReader(filePath);
    }

    bool isSupported(const std::filesystem::path& filePath) const
    {
        return _registry->isSupported(filePath);
    }

    std::vector<std::string> getSupportedExtensions() const
    {
        return _registry->getSupportedExtensions();
    }

    const ReaderRegistry& getRegistry() const
    {
        return *_registry;
    }

    bool isEmpty() const
    {
        return _registry->isEmpty();
    }

    size_t size() const
    {
        return _registry->size();
    }

private:
    std::unique_ptr<ReaderRegistry> _registry;
};

#endif // FILEREADER_H