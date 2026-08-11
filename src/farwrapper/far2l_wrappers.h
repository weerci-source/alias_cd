#pragma once

#include <farplug-wide.h>
#include <expected>
#include <system_error>
#include <string>
#include <vector>

namespace far2l
{
    // Используем точную сигнатуру из SDK
    using ControlFunc = FARAPICONTROL;   // int (*)(HANDLE, int, int, LONG_PTR)
    using MessageFunc = FARAPIMESSAGE;   // при необходимости тоже можно заменить

    std::expected<void, std::error_code> control(HANDLE hPanel, int command, int param1, void *param2,
                                                 ControlFunc controlFunc) noexcept;

    std::expected<void, std::error_code> message(int moduleNumber, int flags, const std::wstring &title,
                                                 const std::vector<std::wstring> &items, int icon,
                                                 MessageFunc msgFunc) noexcept;

    bool isValidHandle(HANDLE h) noexcept;
}