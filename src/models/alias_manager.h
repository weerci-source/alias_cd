#pragma once

#include "alias.h"
#include "../utils/writer.h"   
#include <vector>
#include <string>
#include <expected>
#include <system_error>

class AliasManager {
public:
    static AliasManager& Instance();

    // Инициализация: путь к файлу и контекст для записи
    std::expected<void, std::error_code> init(const std::wstring& filePath) noexcept;

    // Загрузка из файла
    std::expected<void, std::error_code> load() noexcept;

    // Сохранение в файл
    std::expected<void, std::error_code> save() noexcept;

    // Добавление/обновление
    std::expected<void, std::error_code> addOrUpdate(const Alias& alias) noexcept;

    // Удаление по имени
    std::expected<void, std::error_code> remove(const std::wstring& name) noexcept;

    // Поиск по имени (возвращает указатель или ошибку)
    std::expected<const Alias*, std::error_code> find(const std::wstring& name) const noexcept;

    // Получение всех алиасов
    const std::vector<Alias>& getAll() const noexcept { return aliases_; }

    // Очистка всех
    std::expected<void, std::error_code> clear() noexcept;

    AliasManager(const AliasManager&) = delete;
    AliasManager& operator=(const AliasManager&) = delete;

private:
    AliasManager() = default;

    std::vector<Alias> aliases_;
    std::wstring filePath_;
    writer::Context ctx_;   // контекст для записи

    // Вспомогательная функция для получения пути по умолчанию
    static std::wstring getDefaultFilePath() noexcept;
};