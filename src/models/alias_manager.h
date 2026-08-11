#pragma once

#include "../interfaces/IAliasStorage.h"
#include "../interfaces/IFileSystem.h"
#include "../interfaces/IWriter.h"
#include "alias.h"
#include <memory>
#include <vector>
#include <string>
#include "../common.h"         

using namespace alias_cd;

class AliasManager : public IAliasStorage {
public:
    // Конструктор принимает ссылки на зависимости (DI)
    AliasManager(IFileSystem& fs, IWriter& writer) noexcept;

    VoidResult init(const std::wstring& filePath) noexcept override;
    VoidResult load() noexcept override;
    VoidResult save() noexcept override;
    VoidResult addOrUpdate(const Alias& alias) noexcept override;
    VoidResult remove(const std::wstring& name) noexcept override;
    std::expected<const Alias*, std::error_code> find(const std::wstring& name) const noexcept override;
    const std::vector<Alias>& getAll() const noexcept override { return aliases_; }
    VoidResult clear() noexcept override;

private:
    IFileSystem& fs_;
    IWriter& writer_;
    std::vector<Alias> aliases_;
    std::wstring filePath_;

    std::wstring getDefaultFilePath() const noexcept;
};