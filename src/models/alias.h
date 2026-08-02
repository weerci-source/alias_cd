#pragma once

#include <string>
#include <expected>
#include <system_error>

struct Alias {
    std::wstring name;
    std::wstring path;
};

// Вспомогательные чистые функции
bool isValidAlias(const Alias& a) noexcept;
std::expected<Alias, std::error_code> parseAliasLine(const std::string& line) noexcept;
std::string serializeAlias(const Alias& a) noexcept;