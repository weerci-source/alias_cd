#pragma once

#include <expected>
#include <system_error>
#include <string>

class IFileSystem
{
public:
    virtual ~IFileSystem() = default;

    virtual std::expected<std::wstring, std::error_code> getCurrentDir() noexcept = 0;
    virtual std::expected<void, std::error_code> setCurrentDir(const std::wstring &path) noexcept = 0;
};