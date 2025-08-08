#include <PIIScanner.h>

std::map<std::string, std::vector<std::string>> RegexStrategy::scan(const std::string& text)
{
    std::map<std::string, std::vector<std::string>> result;

    for (const auto& [type, regexList]: _cmpPatterns)
    {
        for (const auto& re: regexList)
        {
            re2::StringPiece input(text);
            std::string matchData;

            while (RE2::FindAndConsume(&input, *re, &matchData)) 
                result[type].push_back(matchData);
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
