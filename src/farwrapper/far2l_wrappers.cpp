#include "far2l_wrappers.h"
#include <iostream>

namespace far2l
{

    bool isValidHandle(HANDLE h) noexcept
    {
        return h != nullptr && h != INVALID_HANDLE_VALUE;
    }

    std::expected<void, std::error_code> control(HANDLE hPanel, int command, int param1, void *param2,
                                                 ControlFunc controlFunc) noexcept
    {
        if (!controlFunc)
            return std::unexpected(std::make_error_code(std::errc::function_not_supported));
        if (!isValidHandle(hPanel))
            return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));

        int result = 0;
        try
        {
            // Приведение void* -> LONG_PTR, как того требует FAR API
            result = controlFunc(hPanel, command, param1, reinterpret_cast<LONG_PTR>(param2));
        }
        catch (...)
        {
            return std::unexpected(std::make_error_code(std::errc::io_error));
        }

        if (result != 0)
            return std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
        return {};
    }

    std::expected<void, std::error_code> message(int moduleNumber, int flags, const std::wstring &title,
                                                 const std::vector<std::wstring> &items, int icon,
                                                 MessageFunc msgFunc) noexcept
    {
        if (!msgFunc)
        {
            return std::unexpected(std::make_error_code(std::errc::function_not_supported));
        }

        std::vector<const wchar_t *> itemPtrs;
        itemPtrs.reserve(items.size());
        for (const auto &s : items)
        {
            itemPtrs.push_back(s.c_str());
        }

        try
        {
            intptr_t result = msgFunc(moduleNumber, flags, title.c_str(), itemPtrs.data(),
                                      static_cast<int>(itemPtrs.size()), icon);
            if (result != 0)
            {
                return std::unexpected(std::make_error_code(std::errc::io_error));
            }
        }
        catch (...)
        {
            return std::unexpected(std::make_error_code(std::errc::io_error));
        }
        return {};
    }

} // namespace far2l