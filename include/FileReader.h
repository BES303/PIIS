#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

// TODO: add chunkHandler
class FileReader
{
public:
    virtual std::string readText(const std::filesystem::path& filePath) = 0;
    virtual ~FileReader() {}
};

class TxtReader : public FileReader
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

class PdfReader : public FileReader
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
                throw std::runtime_error("Failed to load PDF: " + utf8Path);

            if (pdf->is_locked())
                throw std::runtime_error("Encrypted PDF: " + filePath.string());

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

class DocReader : public FileReader
{
public:
    std::string readText(const std::filesystem::path& filePath) override
    {
        throw std::runtime_error("DOC not supported");
    }
};


//  ....jpg

class ReaderFactory
{
public:
    static std::unique_ptr<FileReader> createReader(const std::filesystem::path& filePath)
    {
        auto extension = filePath.extension().string();

        if (extension == ".txt")
            return std::make_unique<TxtReader>();
        else if (extension == ".pdf")
            return std::make_unique<PdfReader>();
        else if (extension == ".doc" || extension == ".docx")
            return std::make_unique<DocReader>();
        else
        {
            std::cerr << "Unsupported file format: " << filePath << std::endl;
            return nullptr;
        }
    }
};

#endif // FILEREADER_H
