#pragma once

#include <mutex>
#include <expected>
#include <iostream>
#include <fstream>
#include <functional>

#include "utilites.h"
#include "formatters.h"

namespace writer
{
	struct Context
	{
		std::ofstream file;
		std::mutex mtx;
		Formatter frm = defaultFormatter;
	};

	Context makeContext();

	// Инициализация для конкретного контекста
	std::expected<void, std::error_code> init(Context& ctx, const std::string& filename);

	std::expected<void, std::error_code> write(Context& ctx, const std::string& msg, Formatter frm = nullptr);

	std::expected<void, std::error_code> write(Context& ctx, const std::vector<std::string>& msgs, Formatter frm = nullptr);

	void close(Context& ctx);

	// Вариант 1: возвращает вектор (с перемещением / RVO)
	std::expected<std::vector<std::string>, std::error_code> readAllLines(const std::string& filename);

	// Вариант 2: заполняет переданный вектор (эффективнее)
	std::expected<void, std::error_code> readAllLines(const std::string& filename, std::vector<std::string>& out);
	
	// Расширение для перезаписи файла
	std::expected<void, std::error_code> openOverwrite(Context& ctx, const std::string& filename) noexcept;
}