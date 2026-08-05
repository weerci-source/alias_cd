#include <string>
#include <chrono>
#include <expected>

// Получить текущую дату и время в формате "YYYY-MM-DD HH:MM:SS"
std::string currentTime();

// Инициализация локали (вызывать один раз при загрузке плагина)
std::expected<void, std::error_code> initLocale();

// Преобразование UTF-8 → wstring
std::wstring UTF8ToWString(const std::string &);

// Преобразование wstring → UTF-8
std::string WStringToUTF8(const std::wstring &);

// Получение текущей директории
std::expected<std::wstring, std::error_code> getCurrentDirW() noexcept;

// Установка текущей директории
std::expected<void, std::error_code> setCurrentDirW(const std::wstring &) noexcept;

// Обрезка пробелов (возвращает новую utf-8 строку, не модифицирует исходную)
std::wstring trim(const std::wstring &);

// Обрезка пробелов (возвращает новую utf-16 строку, не модифицирует исходную)
std::string trim(const std::string &);

// Указатели для переопределения в тестах
extern std::expected<std::wstring, std::error_code> (*g_getCurrentDirW_impl)() noexcept;
extern std::expected<void, std::error_code> (*g_setCurrentDirW_impl)(const std::wstring&) noexcept;
