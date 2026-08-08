#pragma once

#include <expected>
#include <system_error>
#include <string>
#include <vector>
#include "../models/alias.h"

class IAliasStorage
{
public:
    virtual ~IAliasStorage() = default;

    virtual std::expected<void, std::error_code> init(const std::wstring &filePath) noexcept = 0;
    virtual std::expected<void, std::error_code> load() noexcept = 0;
    virtual std::expected<void, std::error_code> save() noexcept = 0;
    virtual std::expected<void, std::error_code> addOrUpdate(const Alias &alias) noexcept = 0;
    virtual std::expected<void, std::error_code> remove(const std::wstring &name) noexcept = 0;
    virtual std::expected<const Alias *, std::error_code> find(const std::wstring &name) const noexcept = 0;
    virtual const std::vector<Alias> &getAll() const noexcept = 0;
    virtual std::expected<void, std::error_code> clear() noexcept = 0;
};