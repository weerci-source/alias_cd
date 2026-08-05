#include "effects.h"
#include "farwrapper/far2l_wrappers.h"
#include <fstream>
#include <mutex>

namespace effects {

    void log(const std::string& msg) {
        static std::mutex logMutex;
        std::lock_guard<std::mutex> lock(logMutex);
        std::ofstream("/tmp/alias_cd.log", std::ios::app) << msg << std::endl;
    }

    std::expected<void, std::error_code> control(const PluginContext& ctx,
                                                 HANDLE h, int cmd, int p1, void* p2) noexcept {
        return far2l::control(h, cmd, p1, p2, ctx.Info.Control);
    }

    std::expected<void, std::error_code> message(const PluginContext& ctx,
                                                 const std::wstring& title,
                                                 const std::vector<std::wstring>& items,
                                                 int flags, int icon) noexcept {
        return far2l::message(ctx.Info.ModuleNumber, flags, title, items, icon, ctx.Info.Message);
    }

    void showError(const PluginContext& ctx, const std::wstring& text) {
        static_cast<void>(message(ctx, L"Alias CD Error", { text }));
    }

    void showInfo(const PluginContext& ctx, const std::wstring& text) {
        static_cast<void>(message(ctx, L"Alias CD", { text }));
    }

    std::expected<void, std::error_code> updateActivePanel(const PluginContext& ctx) noexcept {
        return control(ctx, PANEL_ACTIVE, FCTL_SETPANELDIR, 0, nullptr)
            .and_then([&]() { return control(ctx, PANEL_ACTIVE, FCTL_UPDATEPANEL, 0, nullptr); })
            .and_then([&]() { return control(ctx, PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, nullptr); });
    }

    std::expected<void, std::error_code> closePlugin(const PluginContext& ctx, HANDLE hPlugin) noexcept {
        return control(ctx, hPlugin, FCTL_CLOSEPLUGIN, 0, nullptr);
    }
}