#include "alias.h"
#include "../utils/utilites.h"   

bool isValidAlias(const Alias& a) noexcept {
    return !a.name.empty() && !a.path.empty();
}

std::expected<Alias, std::error_code> parseAliasLine(const std::string& line) noexcept {
    if (line.empty() || line[0] == '#') {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    size_t pos = line.find('=');
    if (pos == std::string::npos) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    std::string namePart = line.substr(0, pos);
    std::string pathPart = line.substr(pos + 1);
    trim(namePart);
    trim(pathPart);

    if (namePart.empty() || pathPart.empty()) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    Alias a;
    a.name = UTF8ToWString(namePart);
    a.path = UTF8ToWString(pathPart);

    if (!isValidAlias(a)) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }
    return a;
}

std::string serializeAlias(const Alias& a) noexcept {
    std::string nameUtf8 = WStringToUTF8(a.name);
    std::string pathUtf8 = WStringToUTF8(a.path);
    return nameUtf8 + "=" + pathUtf8;
}