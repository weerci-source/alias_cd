#include "effects.h"
#include <fstream>
#include <mutex>

void Effects::log(const std::string &msg) const
{
    static std::mutex logMutex;
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream("/tmp/alias_cd.log", std::ios::app) << msg << std::endl;
}

std::expected<void, std::error_code> Effects::control(const PluginContext &ctx,
                                                      HANDLE h, int cmd, int p1, void *p2) const noexcept
{
    return farApi_.control(h, cmd, p1, p2);
}

std::expected<void, std::error_code> Effects::message(const PluginContext &ctx,
                                                      const std::wstring &title,
                                                      const std::vector<std::wstring> &items,
                                                      int flags, int icon) const noexcept
{
    return farApi_.message(title, items, flags, icon);
}

void Effects::showError(const PluginContext &ctx, const std::wstring &text) const
{
    static_cast<void>(message(ctx, L"Alias CD Error", {text}));
}

void Effects::showInfo(const PluginContext &ctx, const std::wstring &text) const
{
    static_cast<void>(message(ctx, L"Alias CD", {text}));
}

std::expected<void, std::error_code> Effects::updateActivePanel(const PluginContext &ctx) const noexcept
{
    return control(ctx, PANEL_ACTIVE, FCTL_SETPANELDIR, 0, nullptr)
        .and_then([&]()
                  { return control(ctx, PANEL_ACTIVE, FCTL_UPDATEPANEL, 0, nullptr); })
        .and_then([&]()
                  { return control(ctx, PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, nullptr); });
}

std::expected<void, std::error_code> Effects::closePlugin(const PluginContext &ctx, HANDLE hPlugin) const noexcept
{
    return control(ctx, hPlugin, FCTL_CLOSEPLUGIN, 0, nullptr);
}