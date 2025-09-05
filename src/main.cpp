#include <iostream>
#include <filesystem>
#include "PIIConfigger.h"
#include "FileReaders.h"
#include "PIIDetector.h"
#include "PIIResultExporter.h"
#include "PIIResultHandler.h"
#include "Scanner.h"

int main(int argc, char* argv[])
{
    PIIConfigger configger(argc, argv);

    if (configger.isHelpRequested())
    {
        configger.showHelp();
        return 0;
    }

    GeneralConfig config = configger.getConfig();

    if (config.inputPath.empty())
    {
        std::cerr << "Error: Must specify a file or directory to scan" << std::endl;
        return 1;
    }

    FileReaderFactory readerFactory;
    readerFactory.registerReader<TxtReader>(".txt"); // use .
    readerFactory.registerReader<PdfReader>(".pdf");
    readerFactory.registerReader<XlsxReader>(".xlsx");
    readerFactory.registerReader<PptxReader>(".pptx");
    readerFactory.registerReader<XmlReader>(std::vector<std::string>{".xml" /*, ".html" ... */ }); // TODO: group to markup reader?
    readerFactory.registerReader<DocxReader>(std::vector<std::string>{/*".doc",*/".docx"});

    //TODO:
    // readerFactory.registerReader<ImageReader>({".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".gif"});

    std::unique_ptr<IStrategyScanner> piiStrategy;

    if (config.strategy == "regex")
        piiStrategy = PIIStrategyHandler::createRegexStrategy(config.patterns);
    else if (config.strategy == "keyword")
        piiStrategy = PIIStrategyHandler::createKeywordStrategy(config.keywords);
    else
        throw std::invalid_argument("Unsupported strategy type: " + config.strategy);

    PIIDetector detector(std::move(piiStrategy));

    std::vector<std::unique_ptr<IPIIResultExporter>> exporters;
    exporters.push_back(std::make_unique<ConsoleExporter>());

    if (!config.outputJson.empty())
        exporters.push_back(std::make_unique<JsonExporter>(config.outputJson));

    PIIResultHandler resultProcessor(std::move(exporters));
    PIIScanner scanner(detector, resultProcessor, readerFactory);

    if (std::filesystem::is_regular_file(config.inputPath))
        scanner.scan(config.inputPath);
    else if (std::filesystem::is_directory(config.inputPath))
        scanner.scan(config.inputPath, config.recursive);

    return 0;
}