#pragma once

#include <expected>
#include <system_error>
#include <string>
#include <vector>
#include <functional>

class IWriter
{
public:
    virtual ~IWriter() = default;

    virtual std::expected<void, std::error_code> init(const std::string &filename) noexcept = 0;
    virtual std::expected<void, std::error_code> write(const std::string &msg) noexcept = 0;
    virtual std::expected<void, std::error_code> write(const std::vector<std::string> &msgs) noexcept = 0;
    virtual void close() noexcept = 0;
    virtual std::expected<std::vector<std::string>, std::error_code> readAllLines(const std::string &filename) noexcept = 0;
    virtual std::expected<void, std::error_code> openOverwrite(const std::string &filename) noexcept = 0;
};