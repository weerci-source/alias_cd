#include "RealWriter.h"

std::expected<void, std::error_code> RealWriter::init(const std::string &filename) noexcept
{
    return writer::init(ctx_, filename);
}

std::expected<void, std::error_code> RealWriter::write(const std::string &msg) noexcept
{
    return writer::write(ctx_, msg);
}

std::expected<void, std::error_code> RealWriter::write(const std::vector<std::string> &msgs) noexcept
{
    return writer::write(ctx_, msgs);
}

void RealWriter::close() noexcept
{
    writer::close(ctx_);
}

std::expected<std::vector<std::string>, std::error_code> RealWriter::readAllLines(const std::string &filename) noexcept
{
    return writer::readAllLines(filename);
}

std::expected<void, std::error_code> RealWriter::openOverwrite(const std::string &filename) noexcept
{
    return writer::openOverwrite(ctx_, filename);
}