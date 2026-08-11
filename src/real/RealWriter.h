#pragma once

#include "../interfaces/IWriter.h"
#include "../utils/writer.h" // используем старый writer::Context, но будем его инкапсулировать

class RealWriter : public IWriter
{
public:
    RealWriter() = default;

    std::expected<void, std::error_code> init(const std::string &filename) noexcept override;
    std::expected<void, std::error_code> write(const std::string &msg) noexcept override;
    std::expected<void, std::error_code> write(const std::vector<std::string> &msgs) noexcept override;
    void close() noexcept override;
    std::expected<std::vector<std::string>, std::error_code> readAllLines(const std::string &filename) noexcept override;
    std::expected<void, std::error_code> openOverwrite(const std::string &filename) noexcept override;

private:
    writer::Context ctx_;
};