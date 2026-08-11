#pragma once

#include <expected>
#include <system_error>

using void_err = std::expected<void, std::error_code>;