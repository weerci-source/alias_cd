#pragma once

#include "interfaces/IFarApi.h"
#include "plugin_context.h"
#include <string>
#include <vector>
#include <expected>
#include <system_error>
#include "common.h"

using namespace alias_cd;

class Effects
{
public:
    explicit Effects(IFarApi &farApi) noexcept : farApi_(farApi) {}

    void log(const std::string &msg) const;

    VoidResult control(const PluginContext &ctx,
                   HANDLE h, int cmd, int p1, void *p2) const noexcept;
    VoidResult message(const PluginContext &ctx,
                   const std::wstring &title,
                   const std::vector<std::wstring> &items,
                   int flags = 0, int icon = 0) const noexcept;

    void showError(const PluginContext &ctx, const std::wstring &text) const;
    void showInfo(const PluginContext &ctx, const std::wstring &text) const;

    VoidResult updateActivePanel(const PluginContext &ctx,
                             const std::wstring &newPath = {}) const noexcept;
    VoidResult closePlugin(const PluginContext &ctx, HANDLE hPlugin) const noexcept;

private:
    IFarApi &farApi_;
};