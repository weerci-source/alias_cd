#include <mutex>
#include <expected>
#include <iostream>
#include <fstream>
#include <functional>

#include "utilites.h"
#include "formatters.h"
#include "writer.h"

namespace writer
{
	Context makeContext()
	{
		return Context();
	}

	std::expected<void, std::error_code> init(Context &ctx, const std::string &filename)
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (ctx.file.is_open())
		{
			ctx.file.close();
		}
		ctx.file.open(filename, std::ios::out | std::ios::app);
		if (!ctx.file.is_open())
		{
			return std::unexpected(
				std::make_error_code(std::errc::no_such_file_or_directory));
		}
		return {};
	}

	std::expected<void, std::error_code> write(Context &ctx, const std::string &msg, Formatter frm)
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.file.is_open())
		{
			std::cerr << "[LOGGER] " << msg << std::endl;
			return std::unexpected(
				std::make_error_code(std::errc::bad_file_descriptor));
		}
		ctx.file << (frm ? frm(msg) : ctx.frm(msg));
		ctx.file.flush();
		if (ctx.file.fail())
		{
			return std::unexpected(
				std::make_error_code(std::errc::io_error));
		}
		return {};
	}

	std::expected<void, std::error_code> write(Context &ctx, const std::vector<std::string> &msgs, Formatter frm)
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (!ctx.file.is_open())
		{
			return std::unexpected(
				std::make_error_code(std::errc::bad_file_descriptor));
		}
		for (const auto &msg : msgs)
		{
			ctx.file << (frm ? frm(msg) : ctx.frm(msg));
		}
		ctx.file.flush();
		if (ctx.file.fail())
		{
			return std::unexpected(
				std::make_error_code(std::errc::io_error));
		}
		return {};
	}

	void close(Context &ctx)
	{
		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (ctx.file.is_open())
		{
			ctx.file.close();
		}
	}

	std::expected<std::vector<std::string>, std::error_code> readAllLines(const std::string &filename)
	{
		std::ifstream inFile(filename);
		if (!inFile.is_open())
		{
			return std::unexpected(
				std::make_error_code(std::errc::no_such_file_or_directory));
		}

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(inFile, line))
		{
			if (!line.empty())
			{ // пропускаем пустые строки (опционально)
				lines.push_back(line);
			}
		}

		if (inFile.fail() && !inFile.eof())
		{
			return std::unexpected(
				std::make_error_code(std::errc::io_error));
		}
		return lines;
	}

	std::expected<void, std::error_code> readAllLines(const std::string &filename, std::vector<std::string> &out)
	{
		std::ifstream inFile(filename);
		if (!inFile.is_open())
		{
			return std::unexpected(
				std::make_error_code(std::errc::no_such_file_or_directory));
		}

		out.clear(); // заменяем содержимое (можно убрать, если нужно дополнять)
		// опционально: out.reserve(...) если знаете размер

		std::string line;
		while (std::getline(inFile, line))
		{
			if (!line.empty())
			{
				out.push_back(line);
			}
		}

		if (inFile.fail() && !inFile.eof())
		{
			return std::unexpected(
				std::make_error_code(std::errc::io_error));
		}
		return {};
	}

	std::expected<void, std::error_code> openOverwrite(Context &ctx, const std::string &filename) noexcept
	{

		std::lock_guard<std::mutex> lock(ctx.mtx);
		if (ctx.file.is_open())
		{
			ctx.file.close();
		}
		ctx.file.open(filename, std::ios::out | std::ios::trunc);
		if (!ctx.file.is_open())
		{
			return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
		}
		return {};
	}
}