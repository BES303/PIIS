#include <iostream>
#include <filesystem>
#include "PIIConfigger.h"
#include "FileReader.h"
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

    if (config.filePath.empty() && config.directoryPath.empty())
    {
        std::cerr << "Error: Must specify a file or directory to scan" << std::endl;
        return 1;
    }

    ReaderFactory readerFactory;
    readerFactory.registerReader<TxtReader>(".txt"); // use .
    readerFactory.registerReader<PdfReader>(".pdf");

    //TODO:
    // readerFactory.registerReaderForExtensions<ImageReader>({".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".gif"});
    // readerFactory.registerReaderForExtensions<DocReader>({".doc", ".docx"});

    auto piiStrategy = PIIStrategyHandler::createStrategy(config.strategy, config.patterns, config.keywords); //

    std::vector<std::unique_ptr<IPIIResultExporter>> exporters;

    exporters.push_back(std::make_unique<ConsoleExporter>());

    if (!config.outputJson.empty())
        exporters.push_back(std::make_unique<JsonExporter>(config.outputJson));

    PIIDetector detector(std::move(piiStrategy));
    PIIResultHandler resultProcessor(std::move(exporters));

    PIIScanner scanner(detector, resultProcessor, readerFactory);

    if (!config.filePath.empty())
        scanner.scan(config.filePath);

    if (!config.directoryPath.empty())
        scanner.scan(config.directoryPath, config.recursive);

    resultProcessor.finalize();

    return 0;
}