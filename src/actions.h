#pragma once

#include <expected>
#include <system_error>
#include <string>
#include <farplug-wide.h>


struct PluginContext;

namespace actions {
    using Result = std::expected<HANDLE, std::error_code>;

    Result openAliasesPanel(const PluginContext& ctx) noexcept;
    Result saveAlias(const PluginContext& ctx, std::wstring aliasName) noexcept;
    Result gotoAlias(const PluginContext& ctx, std::wstring aliasName) noexcept;
    Result processOpenCommand(const PluginContext& ctx, const std::wstring& cmdLine) noexcept;
}