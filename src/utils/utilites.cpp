#include <string>
#include <chrono>

#include "utilites.h"
#include <expected>
#include <filesystem>

std::string currentTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
    localtime_r(&in_time_t, &bt);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::expected<void, std::error_code> initLocale()
{
    const char *result = std::setlocale(LC_ALL, "en_US.UTF-8");
    if (result == nullptr)
    {
        // Не удалось установить локаль – возвращаем ошибку
        return std::unexpected(
            std::make_error_code(std::errc::invalid_argument));
    }
    return {};
}

std::wstring UTF8ToWString(const std::string &utf8)
{
    if (utf8.empty())
        return {};
    size_t len = mbstowcs(nullptr, utf8.c_str(), 0);
    if (len == (size_t)-1)
        return {};
    std::wstring result(len, L'\0');
    mbstowcs(&result[0], utf8.c_str(), len + 1);
    return result;
}

std::string WStringToUTF8(const std::wstring &wstr)
{
    if (wstr.empty())
        return {};
    size_t len = wcstombs(nullptr, wstr.c_str(), 0);
    if (len == (size_t)-1)
        return {};
    std::string result(len, '\0');
    wcstombs(&result[0], wstr.c_str(), len + 1);
    return result;
}

std::expected<std::wstring, std::error_code> getCurrentDirW() noexcept
{
    std::filesystem::path current;
    try
    {
        current = std::filesystem::current_path();
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        // Преобразуем исключение в error_code
        return std::unexpected(e.code());
    }
    catch (...)
    {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
    return current.wstring(); // или UTF8ToWString(current.string())
}

std::expected<void, std::error_code> setCurrentDirW(const std::wstring &path) noexcept
{
    try
    {
        std::filesystem::current_path(path);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        return std::unexpected(e.code());
    }
    catch (...)
    {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
    return {};
}

std::wstring trim(const std::wstring &s)
{
    auto first = s.find_first_not_of(L" \t\n\r");
    if (first == std::wstring::npos)
    {
        return {};
    }
    auto last = s.find_last_not_of(L" \t\n\r");
    return s.substr(first, last - first + 1);
}

std::string trim(const std::string &s)
{
    auto first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
    {
        return {};
    }
    auto last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}
