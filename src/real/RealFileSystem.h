#pragma once

#include "../interfaces/IFileSystem.h"

class RealFileSystem : public IFileSystem
{
public:
    std::expected<std::wstring, std::error_code> getCurrentDir() noexcept override;
    std::expected<void, std::error_code> setCurrentDir(const std::wstring &path) noexcept override;
};