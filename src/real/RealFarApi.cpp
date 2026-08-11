#include "RealFarApi.h"

RealFarApi::RealFarApi(INT_PTR moduleNumber, FARAPICONTROL controlFunc, FARAPIMESSAGE msgFunc) noexcept
    : moduleNumber_(moduleNumber), controlFunc_(controlFunc), msgFunc_(msgFunc) {}

std::expected<void, std::error_code> RealFarApi::control(HANDLE h, int cmd, int p1, void *p2) noexcept
{
    return far2l::control(h, cmd, p1, p2, controlFunc_);
}

std::expected<void, std::error_code> RealFarApi::message(const std::wstring &title,
                                                         const std::vector<std::wstring> &items,
                                                         int flags, int icon) noexcept
{
    return far2l::message(moduleNumber_, flags, title, items, icon, msgFunc_);
}