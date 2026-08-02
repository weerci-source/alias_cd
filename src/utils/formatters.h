#pragma once

#include <string_view>
#include <string>

#include "utilites.h"

namespace writer
{

    using Formatter = std::function<std::string(std::string_view)>;

    inline auto logFormatter = [](std::string_view level)
    {
        return [level](std::string_view msg)
        {
            return "[" + currentTime() + "] " + std::string(level) + ": " + std::string(msg);
        };
    };

    inline auto defaultFormatter = [](std::string_view msg)
    {
            return std::string(msg) + "\n";
    };
}