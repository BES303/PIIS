#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>


class FileReader
{
    
protected:
    std::string filePath;

public:
    virtual std::string readText(const std::string& filePath) = 0;
    virtual ~FileReader() {}
};

class TxtReader : public FileReader
{
public:
    std::string readText(const std::string& filePath) override
    {
        std::ifstream file(filePath);

        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + filePath);

        std::string fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return fileData;
    }
};

class PdfReader : public FileReader
{
public:
    std::string readText(const std::string& filePath) override
    {
        try
        {
            std::unique_ptr<poppler::document> pdf(poppler::document::load_from_file(filePath));
            
            if (!pdf)
                throw std::runtime_error("Failed to load PDF file: " + filePath);

            if (pdf->is_locked())
                return "";

            std::string text;
            const int pages = pdf->pages();

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
    std::string readText(const std::string& filePath) override
    {
        throw std::runtime_error("DOC not supported");
    }
};


//  ....jpg

class FileReaderFactory
{
public:
    static std::unique_ptr<FileReader> createReader(const std::string& filePath)
    {
        auto extension = std::filesystem::path(filePath).extension().string();

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
