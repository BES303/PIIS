#include "FileReaders.h"
#include "GeneralConfig.h"

#include <xlnt/xlnt.hpp>
#include <iostream>
#include <fstream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

// TODO: UTF-8

void checkFile(const std::filesystem::path& filePath, size_t maxSize)
{
    if (!std::filesystem::exists(filePath))
        throw std::invalid_argument("File does not exist: " + filePath.string());

    const auto fileSize = std::filesystem::file_size(filePath);
    if (fileSize > maxSize)
        throw std::runtime_error(filePath.string() + " file exceeds maximum size (" +
                                 std::to_string(fileSize) + " > " + std::to_string(maxSize) + ")");
}

std::string TxtReader::readText(const std::filesystem::path& filePath)
{
    checkFile(filePath, GeneralConfig::MAX_TXT_SIZE);

    std::ifstream file(filePath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open file: " + filePath.string());

    std::string fileData;
    const auto fileSize = std::filesystem::file_size(filePath);
    fileData.reserve(static_cast<size_t>(fileSize));
    fileData.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return fileData;
}

std::string PptxReader::readText(const std::filesystem::path& filePath)
{
    checkFile(filePath, GeneralConfig::MAX_PPTX_SIZE);

    try
    {
        libzippp::ZipArchive zip(filePath.string().c_str());
        std::string resultData;
        resultData.reserve(static_cast<size_t>(GeneralConfig::MAX_PPTX_SIZE));
        extractPptxData(zip, resultData);
        return resultData;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("PPTX processing error: " + std::string(e.what()));
    }
}

void PptxReader::extractPptxData(libzippp::ZipArchive& zip, std::string& resultData)
{
    if (!zip.open(libzippp::ZipArchive::ReadOnly))
        throw std::runtime_error("Failed to open PPTX archive");

    try
    {
        std::vector<libzippp::ZipEntry> zipEntries = zip.getEntries();
        for (const auto& zipEntry: zipEntries)
        {
            const auto zipEntryName = zipEntry.getName();
            if (zipEntryName.find("ppt/slides/slide") != std::string::npos &&
                zipEntryName.find(".xml") != std::string::npos)
            {
                if (zipEntryName.find(".rels") != std::string::npos)
                    continue;

                const auto xmlData = zipEntry.readAsText();
                extractSlideData(xmlData, resultData);
            }
        }
    }
    catch (...)
    {
        zip.close();
        throw;
    }
    zip.close();
}

void PptxReader::extractSlideData(const std::string& xmlData, std::string& resultData)
{
    pugi::xml_document xmlDoc;
    const auto parseResult = xmlDoc.load_string(xmlData.c_str());
    if (!parseResult)
        throw std::runtime_error(std::string("Failed to parse slide XML: ") + parseResult.description());

    auto slideNode = [&xmlDoc]() -> pugi::xml_node
    {
        constexpr std::string_view tags[] = {
            "p:sld",
            "p:notes",
            "p:handoutMaster"
        };

        for (const auto& tag: tags)
            if (auto node = xmlDoc.child(tag.data()); node)
                return node;

        return xmlDoc;
    }();

    processSlideNode(slideNode, resultData);
}

void PptxReader::processSlideNode(const pugi::xml_node& node, std::string& resultData)
{
    if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata)
    {
        const char* text = node.value();
        if (text && *text)
            resultData += text;
        return;
    }

    auto nodeName = std::string(node.name());
    if (nodeName.find(':') != std::string::npos)
        nodeName = nodeName.substr(nodeName.find(':') + 1);

    if (nodeName == "t")
    {
        const char* text = node.text().get();
        if (text && *text)
            resultData += text;
        return;
    }

    if (node.children().begin() != node.children().end())
        for (const auto& child : node.children())
            processSlideNode(child, resultData);
}

std::string PdfReader::readText(const std::filesystem::path& filePath)
{
    checkFile(filePath, GeneralConfig::MAX_PDF_SIZE);

    try
    {
        const auto u8Path = filePath.u8string();
        std::string utf8Path(u8Path.begin(), u8Path.end());
        auto pdf = poppler::document::load_from_file(utf8Path);

        if (!pdf)
            throw std::runtime_error("Failed to load PDF: " + filePath.string());

        if (pdf->is_locked())
            throw std::runtime_error("Encrypted PDF not supported: " + filePath.string());

        std::string resultData;
        resultData.reserve(static_cast<size_t>(GeneralConfig::MAX_PDF_SIZE));

        const auto numPages = pdf->pages();

        for (size_t page = 0; page < numPages; ++page)
        {
            auto pageData = pdf->create_page(static_cast<int>(page));
            if (pageData)
            {
                const auto byteArray = pageData->text().to_utf8();
                if (!byteArray.empty())
                    resultData.append(byteArray.begin(), byteArray.end());
            }
        }

        return resultData;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("PDF processing error: " + std::string(e.what()));
    }
}

std::string DocxReader::readText(const std::filesystem::path& filePath)
{
    checkFile(filePath, GeneralConfig::MAX_DOCX_SIZE);

    try
    {
        libzippp::ZipArchive zip(filePath.string().c_str());
        std::string resultData;

        resultData.reserve(static_cast<size_t>(GeneralConfig::MAX_DOCX_SIZE)); // TODO:

        extractDocxData(zip, resultData);
        return resultData;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("DOCX processing error: " + std::string(e.what()));
    }
}

void DocxReader::extractDocxData(libzippp::ZipArchive& zip, std::string& resultData)
{
    if (!zip.open(libzippp::ZipArchive::ReadOnly))
        throw std::runtime_error("Failed to open DOCX archive");

    try
    {
        auto entryFile = zip.getEntry("word/document.xml");
        if (entryFile.isNull())
            throw std::runtime_error("document.xml not found in DOCX");

        const auto fileSize = entryFile.getSize();
        auto xmlData = entryFile.readAsText();

        if (static_cast<libzippp_uint64>(xmlData.size()) != fileSize)
            throw std::runtime_error("Failed to read full document.xml content");

        extractXmlData(xmlData, resultData);
    }
    catch (...)
    {
        zip.close();
        throw;
    }
    zip.close();
}

void DocxReader::extractXmlData(const std::string& xmlData, std::string& resultData)
{
    pugi::xml_document xmlDoc;
    const auto parseResult = xmlDoc.load_string(xmlData.c_str(), pugi::parse_default | pugi::parse_escapes);

    if (!parseResult)
        throw std::runtime_error("Failed to parse document.xml: " + std::string(parseResult.description()));

    auto xmlBody = xmlDoc.child("w:document").child("w:body");

    if (!xmlBody)
        throw std::runtime_error("Invalid DOCX structure: missing body element");

    for (const auto& child: xmlBody.children())
        processXmlNode(child, resultData);
}

void DocxReader::processXmlNode(const pugi::xml_node& node, std::string& resultData)
{
    const auto nodeName = node.name();

    if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata)
    {
        resultData += node.value();
        return;
    }

    if ((std::strcmp(nodeName, "w:p") == 0 || std::strcmp(nodeName, "wp:p") == 0) && (!resultData.empty() && resultData.back() != '\n'))
            resultData += '\n';
    else if (std::strcmp(nodeName, "w:tab") == 0 || std::strcmp(nodeName, "wp:tab") == 0)
        resultData += '\t';
    else if (std::strcmp(nodeName, "w:br") == 0 || std::strcmp(nodeName, "wp:br") == 0)
        resultData += '\n';

    for (const auto& child: node.children())
        processXmlNode(child, resultData);

    if ((std::strcmp(nodeName, "w:r") == 0 || std::strcmp(nodeName, "wp:r") == 0 ||
        std::strcmp(nodeName, "w:p") == 0 || std::strcmp(nodeName, "wp:p") == 0) &&
        (!resultData.empty() && resultData.back() != ' ' && resultData.back() != '\n' && resultData.back() != '\t'))
    {
            resultData += ' ';
    }
}

std::string XmlReader::readText(const std::filesystem::path& filePath)
{
    checkFile(filePath, GeneralConfig::MAX_XML_SIZE);
    try
    {
        std::ifstream file(filePath, std::ios::binary);
        std::string xmlData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string resultData;
        resultData.reserve(static_cast<size_t>(GeneralConfig::MAX_XML_SIZE));   // TODO:
        extractDataFromXml(xmlData, resultData);
        return resultData;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("XML processing error: " + std::string(e.what()));
    }
}

void XmlReader::extractDataFromXml(const std::string& xmlData, std::string& resultData)
{
    pugi::xml_document xmlDoc;
    if (!xmlDoc.load_string(xmlData.c_str()))
        throw std::runtime_error("Failed to parse XML file");

    processXmlNode(xmlDoc, resultData);
}

void XmlReader::processXmlNode(const pugi::xml_node& node, std::string& resultData)
{
    for (const auto& child: node.children())
    {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata)
            resultData += child.value();
        else if (child.type() == pugi::node_element)
        {
            if (!resultData.empty() && resultData.back() != ' ' && resultData.back() != '\n' && resultData.back() != '\t')
                resultData += ' ';
            processXmlNode(child, resultData);
        }
    }
}

std::string XlsxReader::readText(const std::filesystem::path& filePath)
{
    checkFile(filePath, GeneralConfig::MAX_XLSX_SIZE);
    try
    {
        xlnt::workbook wb;
        wb.load(filePath.string());
        std::string resultData;
        resultData.reserve(static_cast<size_t>(GeneralConfig::MAX_XLSX_SIZE) * 2);  // TODO:
        for (std::size_t i = 0; i < wb.sheet_count(); ++i)
        {
            auto ws = wb.sheet_by_index(i);
            std::string sheetString;

            for (auto row: ws.rows(false))
            {
                std::string rowString;
                for (auto cell: row)
                {
                    if (!cell.has_value())
                        continue;
                    auto cellText = cell.to_string();

                    if (cellText.empty())
                        continue;

                    if (!rowString.empty())
                        rowString += ' ';
                    rowString += cellText;
                }

                if (!rowString.empty())
                {
                    if (!sheetString.empty())
                        sheetString += '\n';
                    sheetString += rowString;
                }
            }

            if (!sheetString.empty())
            {
                if (!resultData.empty())
                    resultData += "\n\n";

                resultData += sheetString;
            }
        }

        return resultData;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("XLSX processing error: " + std::string(e.what()));
    }
}
