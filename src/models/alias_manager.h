#pragma once

#include "../interfaces/IAliasStorage.h"
#include "../interfaces/IFileSystem.h"
#include "../interfaces/IWriter.h"
#include "alias.h"
#include <memory>
#include <vector>
#include <string>

class AliasManager : public IAliasStorage {
public:
    // Конструктор принимает ссылки на зависимости (DI)
    AliasManager(IFileSystem& fs, IWriter& writer) noexcept;

    std::expected<void, std::error_code> init(const std::wstring& filePath) noexcept override;
    std::expected<void, std::error_code> load() noexcept override;
    std::expected<void, std::error_code> save() noexcept override;
    std::expected<void, std::error_code> addOrUpdate(const Alias& alias) noexcept override;
    std::expected<void, std::error_code> remove(const std::wstring& name) noexcept override;
    std::expected<const Alias*, std::error_code> find(const std::wstring& name) const noexcept override;
    const std::vector<Alias>& getAll() const noexcept override { return aliases_; }
    std::expected<void, std::error_code> clear() noexcept override;

private:
    IFileSystem& fs_;
    IWriter& writer_;
    std::vector<Alias> aliases_;
    std::wstring filePath_;

    std::wstring getDefaultFilePath() const noexcept;
};