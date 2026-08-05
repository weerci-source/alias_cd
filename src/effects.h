#pragma once

#include <expected>
#include <system_error>
#include <string>
#include <vector>
#include <farplug-wide.h>

#include "plugin_context.h"

namespace effects {

    // Указатели для переопределения в тестах (по умолчанию nullptr)
    inline std::expected<void, std::error_code> (*g_control_impl)(const PluginContext&, HANDLE, int, int, void*) = nullptr;
    inline std::expected<void, std::error_code> (*g_message_impl)(const PluginContext&, const std::wstring&, const std::vector<std::wstring>&, int, int) = nullptr;

    void log(const std::string& msg);

    std::expected<void, std::error_code> control(const PluginContext& ctx,
                                                 HANDLE h, int cmd, int p1, void* p2) noexcept;

    std::expected<void, std::error_code> message(const PluginContext& ctx,
                                                 const std::wstring& title,
                                                 const std::vector<std::wstring>& items,
                                                 int flags = 0, int icon = 0) noexcept;

    void showError(const PluginContext& ctx, const std::wstring& text);
    void showInfo(const PluginContext& ctx, const std::wstring& text);

    std::expected<void, std::error_code> updateActivePanel(const PluginContext& ctx) noexcept;
    std::expected<void, std::error_code> closePlugin(const PluginContext& ctx, HANDLE hPlugin) noexcept;
}