#pragma once

#include "../interfaces/IFarApi.h"
#include "../farwrapper/far2l_wrappers.h"

class RealFarApi : public IFarApi
{
public:
    RealFarApi(INT_PTR moduleNumber, FARAPICONTROL controlFunc, FARAPIMESSAGE msgFunc) noexcept;

    std::expected<void, std::error_code> control(HANDLE h, int cmd, int p1, void *p2) noexcept override;
    std::expected<void, std::error_code> message(const std::wstring &title,
                                                 const std::vector<std::wstring> &items,
                                                 int flags = 0, int icon = 0) noexcept override;

private:
    INT_PTR moduleNumber_;
    FARAPICONTROL controlFunc_;
    FARAPIMESSAGE msgFunc_;
};