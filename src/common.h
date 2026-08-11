#pragma once

#include <expected>
#include <system_error>
#include <string>

namespace alias_cd {

// Алиас для результата операций
using VoidResult = std::expected<void, std::error_code>;

// Путь к лог-файлу
inline const std::string LOG_FILE_PATH = "/tmp/alias_cd.log";

// Имя каталога конфигурации FAR (в домашней папке)
inline const std::string FAR_CONFIG_DIR = ".far2l";

// Имя файла алиасов
inline const std::string ALIASES_FILE_NAME = "aliases";

} // namespace alias_cd