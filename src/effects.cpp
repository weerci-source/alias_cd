#include "effects.h"
#include "common.h"
#include <fstream>
#include <mutex>


void Effects::log(const std::string &msg) const
{
    static std::mutex logMutex;
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream(LOG_FILE_PATH, std::ios::app) << msg << std::endl;
}

VoidResult Effects::control(const PluginContext &ctx,
                        HANDLE h, int cmd, int p1, void *p2) const noexcept
{
    return farApi_.control(h, cmd, p1, p2);
}

VoidResult Effects::message(const PluginContext &ctx,
                        const std::wstring &title,
                        const std::vector<std::wstring> &items,
                        int flags, int icon) const noexcept
{
    return farApi_.message(title, items, flags, icon);
}

void Effects::showError(const PluginContext &ctx, const std::wstring &text) const
{
    log("Show error: " + std::string(text.begin(), text.end()));
    static_cast<void>(message(ctx, L"Alias CD Error", {text}));
}

void Effects::showInfo(const PluginContext &ctx, const std::wstring &text) const
{
    static_cast<void>(message(ctx, L"Alias CD", {text}));
}

VoidResult Effects::updateActivePanel(const PluginContext &ctx,
                                  const std::wstring &newPath) const noexcept
{
    return farApi_.control(PANEL_ACTIVE, FCTL_SETPANELDIR, 0, (void *)newPath.c_str())
        .and_then([this]()
                  { return farApi_.control(PANEL_ACTIVE, FCTL_UPDATEPANEL, 0, 0); })
        .and_then([this]()
                  { return farApi_.control(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, 0); })
        .or_else([this](const std::error_code &ec)
                 {
            log("Failed to update active panel: " + ec.message());
            return std::expected<void, std::error_code>{}; });
}

VoidResult Effects::closePlugin(const PluginContext &ctx, HANDLE hPlugin) const noexcept
{
    return control(ctx, hPlugin, FCTL_CLOSEPLUGIN, 0, nullptr);
}