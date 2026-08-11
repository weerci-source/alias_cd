#include "alias_manager.h"
#include "../utils/utilites.h"
#include "../utils/writer.h"
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <iostream>
#include <algorithm> 

AliasManager::AliasManager(IFileSystem &fs, IWriter &writer) noexcept
    : fs_(fs), writer_(writer) {}

std::wstring AliasManager::getDefaultFilePath() const noexcept
{
    const char *home = getenv("HOME");
    if (!home)
    {
        struct passwd *pw = getpwuid(getuid());
        if (pw)
            home = pw->pw_dir;
    }
    if (!home)
    {
        writer::logToStderrAndFile("Could not determine home directory");
        return L"";
    }
    // Используем вынесенные константы
    std::string dir = std::string(home) + "/" + FAR_CONFIG_DIR;
    mkdir(dir.c_str(), 0755);
    return UTF8ToWString(dir + "/" + ALIASES_FILE_NAME);
}

VoidResult AliasManager::init(const std::wstring &filePath) noexcept
{
    filePath_ = filePath.empty() ? getDefaultFilePath() : filePath;
    if (filePath_.empty())
    {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }
    return {};
}

VoidResult AliasManager::load() noexcept
{
    if (filePath_.empty())
    {
        auto initRes = init(L"");
        if (!initRes)
            return std::unexpected(initRes.error());
    }
    std::string pathUtf8 = WStringToUTF8(filePath_);
    auto linesRes = writer_.readAllLines(pathUtf8);
    if (!linesRes)
    {
        if (linesRes.error() == std::make_error_code(std::errc::no_such_file_or_directory))
        {
            aliases_.clear();
            return {};
        }
        return std::unexpected(linesRes.error());
    }
    aliases_.clear();
    for (const auto &line : *linesRes)
    {
        auto aliasRes = parseAliasLine(line);
        if (aliasRes)
        {
            aliases_.push_back(*aliasRes);
        }
        else
        {
            writer::logToStderrAndFile("Skipping invalid alias line: " + line);
        }
    }
    return {};
}

VoidResult AliasManager::save() noexcept
{
    if (filePath_.empty())
    {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }
    std::string pathUtf8 = WStringToUTF8(filePath_);
    auto reopenRes = writer_.openOverwrite(pathUtf8);
    if (!reopenRes)
        return std::unexpected(reopenRes.error());
    for (const auto &alias : aliases_)
    {
        std::string line = serializeAlias(alias);
        auto writeRes = writer_.write(line);
        if (!writeRes)
            return std::unexpected(writeRes.error());
    }
    return {};
}

VoidResult AliasManager::addOrUpdate(const Alias &alias) noexcept
{
    if (!isValidAlias(alias))
    {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }
    for (auto &a : aliases_)
    {
        if (a.name == alias.name)
        {
            a.path = alias.path;
            return save();
        }
    }
    aliases_.push_back(alias);
    return save();
}

VoidResult AliasManager::remove(const std::wstring &name) noexcept
{
    auto it = std::find_if(aliases_.begin(), aliases_.end(),
                           [&name](const Alias &a)
                           { return a.name == name; });
    if (it == aliases_.end())
    {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }
    aliases_.erase(it);
    return save();
}

std::expected<const Alias *, std::error_code> AliasManager::find(const std::wstring &name) const noexcept
{
    for (const auto &a : aliases_)
    {
        if (a.name == name)
            return &a;
    }
    return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
}

VoidResult AliasManager::clear() noexcept
{
    aliases_.clear();
    return save();
}