#include "RealFileSystem.h"
#include <filesystem>

std::expected<std::wstring, std::error_code> RealFileSystem::getCurrentDir() noexcept
{
    try
    {
        return std::filesystem::current_path().wstring();
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        return std::unexpected(e.code());
    }
    catch (...)
    {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
}

std::expected<void, std::error_code> RealFileSystem::setCurrentDir(const std::wstring &path) noexcept
{
    try
    {
        std::filesystem::current_path(path);
        return {};
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        return std::unexpected(e.code());
    }
    catch (...)
    {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
}