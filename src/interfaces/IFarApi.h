#pragma once

#include <expected>
#include <system_error>
#include <string>
#include <vector>
#include <farplug-wide.h>

class IFarApi
{
public:
    virtual ~IFarApi() = default;

    virtual std::expected<void, std::error_code> control(
        HANDLE h, int cmd, int p1, void *p2) noexcept = 0;

    virtual std::expected<void, std::error_code> message(
        const std::wstring &title,
        const std::vector<std::wstring> &items,
        int flags = 0, int icon = 0) noexcept = 0;
};