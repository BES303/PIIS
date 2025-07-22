#include <PIIScanner.h>

std::map<std::string, std::vector<std::string>> RegexStrategy::scan(const std::string& text)
{
    std::map<std::string, std::vector<std::string>> result;

    for (const auto& [type, regexList] : _patterns)
    {
        for (const auto& re : regexList)
        {
            try
            {
                std::sregex_iterator begin(text.begin(), text.end(), re);
                std::sregex_iterator end;

                for (auto it = begin; it != end; ++it)
                    result[type].push_back(it->str());
            }
            catch (const std::regex_error& e)
            {
                std::cerr << "Regex error for pattern " << type << ": " << e.what() << std::endl;
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
            while ((pos = lowerText.find(keyword, pos)) != std::string::npos)
            {
                if ((pos == 0 || !std::isalnum(lowerText[pos - 1])) &&
                    (pos + keyword.size() == lowerText.size() || !std::isalnum(lowerText[pos + keyword.size()])))
                {
                    result[category].push_back(text.substr(pos, keyword.size()));
                }
                pos += keyword.size();
            }
        }
    }

    return result;
}