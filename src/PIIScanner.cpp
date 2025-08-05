#include <PIIScanner.h>
#include <re2/re2.h>

std::map<std::string, std::vector<std::string>> RegexStrategy::scan(const std::string& text)
{
    std::map<std::string, std::vector<std::string>> result;

    for (const auto& [type, regexList] : _patterns)
    {
        for (const auto& pattern : regexList)
        {
            try
            {
                re2::RE2 re(pattern);

                if (!re.ok())
                {
                    std::cerr << "RegexStrategy: Invalid regex pattern for type '" << type << "': " << re.error() << std::endl;
                    continue;
                }

                re2::StringPiece input(text);
                std::string matchData;
                
                while (RE2::FindAndConsume(&input, re, &matchData))
                    result[type].push_back(matchData);

            }
            catch (const std::exception& e)
            {
                std::cerr << "RegexStrategy: Error processing pattern type '" << type << "': " << e.what() << std::endl;
            }
        }
    }

    return result;
}

std::map<std::string, std::vector<std::string>> KeywordStrategy::scan(const std::string& text)
{
    std::map<std::string, std::vector<std::string>> result;
    std::string lowerText = text;

    for (const auto& [category, keywords] : _lowerKeywords)
    {
        for (const auto& keyword : keywords)
        {
            size_t pos = 0;
            size_t matchCount = 0;
            while ((pos = lowerText.find(keyword, pos)) != std::string::npos)
            {
                if ((pos == 0 || !std::isalnum(lowerText[pos - 1])) &&
                    (pos + keyword.size() == lowerText.size() || !std::isalnum(lowerText[pos + keyword.size()])))
                {
                    result[category].push_back(text.substr(pos, keyword.size()));
                    matchCount++;
                }
                pos += keyword.size();
            }
        }
    }

    return result;
}
