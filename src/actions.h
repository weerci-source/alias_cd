#pragma once

#include "interfaces/IAliasStorage.h"
#include "interfaces/IFileSystem.h"
#include "effects.h"
#include "plugin_context.h"
#include <expected>
#include <system_error>
#include <string>
#include <farplug-wide.h>

class Actions
{
public:
    Actions(IAliasStorage &storage, IFileSystem &fs, const Effects &effects) noexcept
        : storage_(storage), fs_(fs), effects_(effects) {}

    using Result = std::expected<HANDLE, std::error_code>;

    Result openAliasesPanel(const PluginContext &ctx) const noexcept;
    Result saveAlias(const PluginContext &ctx, std::wstring aliasName) const noexcept;
    Result gotoAlias(const PluginContext &ctx, std::wstring aliasName) const noexcept;
    Result processOpenCommand(const PluginContext &ctx, const std::wstring &cmdLine) const noexcept;

private:
    IAliasStorage &storage_;
    IFileSystem &fs_;
    const Effects &effects_;
};