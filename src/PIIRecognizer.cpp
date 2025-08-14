#include <PIIRecognizer.h>

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

std::map<std::string, std::vector<std::string>> KeywordStrategy::scan(const std::string& text) // TODO: opt
{
    std::map<std::string, std::vector<std::string>> result;

    if (text.empty())
        return result;

    auto isBoundary = [&text](size_t pos, size_t len) -> bool
    {
            if (pos > 0)
            {
                char prevChar = text[pos - 1];
                if (std::isalnum(static_cast<unsigned char>(prevChar)) || prevChar == '_')
                    return false;
            }

            if (pos + len < text.size())
            {
                char nextChar = text[pos + len];
                if (std::isalnum(static_cast<unsigned char>(nextChar)) || nextChar == '_')
                    return false;
            }

            return true;
    };

    for (const auto& [category, keywords] : _lowerKeywords)
    {
        for (const auto& keyword : keywords)
        {
            if (keyword.empty())
                continue;

            const size_t keywordSize = keyword.size();
            size_t pos = 0;

            while (pos + keywordSize <= text.size())
            {
                bool match = true;
                for (size_t i = 0; i < keywordSize; ++i)
                {
                    int textCharLower = std::tolower(static_cast<unsigned char>(text[pos + i]));
                    int keywordChar = static_cast<unsigned char>(keyword[i]);

                    if (textCharLower != keywordChar)
                    {
                        match = false;
                        break;
                    }
                }

                if (match && isBoundary(pos, keywordSize))
                    result[category].emplace_back(text.begin() + pos, text.begin() + pos + keywordSize);

                ++pos;
            }
        }
    }

    return result;
}