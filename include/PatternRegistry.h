#ifndef PATTERNREGISTRY_H
#define PATTERNREGISTRY_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <filesystem>
#include <memory>

class IPatternProvider
{
public:
    virtual ~IPatternProvider() = default;
    virtual void provide(std::map<std::string, std::vector<std::string>>& patterns,
                        std::map<std::string, std::vector<std::string>>& keywords) = 0;
};

class PatternRegistry
{
public:
    void registryProvider(const std::string& type, std::function<std::unique_ptr<IPatternProvider>()> creator);
    void loadPatterns(const std::string& providerType, const std::filesystem::path& configFile = "");
    const auto& getPatterns() const { return _patterns; }
    const auto& getKeywords() const { return _keywords; }

private:
    std::map<std::string, std::function<std::unique_ptr<IPatternProvider>()>> _providers;
    std::map<std::string, std::vector<std::string>> _patterns;
    std::map<std::string, std::vector<std::string>> _keywords;
};

class DefaultProvider: public IPatternProvider
{
public:
    void provide(std::map<std::string, std::vector<std::string>>& patterns,
                 std::map<std::string, std::vector<std::string>>& keywords) override;
};

class JsonProvider: public IPatternProvider
{
public:
    JsonProvider() = default;
    explicit JsonProvider(const std::string& configFile): _configFile(configFile) {}

    void provide(std::map<std::string, std::vector<std::string>>& patterns,
                 std::map<std::string, std::vector<std::string>>& keywords) override;


private:
    std::string _configFile;
};


#endif // PATTERNREGISTRY_H