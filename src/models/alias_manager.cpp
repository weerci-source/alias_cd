#include "alias_manager.h"
#include "../utils/writer.h"
#include "../utils/utilites.h"
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <iostream>

AliasManager& AliasManager::Instance() {
	static AliasManager instance;
	return instance;
}

std::wstring AliasManager::getDefaultFilePath() noexcept {
	const char* home = getenv("HOME");
	if (!home) {
		struct passwd* pw = getpwuid(getuid());
		if (pw) home = pw->pw_dir;
	}
	if (!home) {
		std::cerr << "Could not determine home directory\n";
		return L"";
	}
	std::string dir = std::string(home) + "/.far2l";
	mkdir(dir.c_str(), 0755);
	return UTF8ToWString(dir + "/aliases");
}

// Инициализация
std::expected<void, std::error_code> AliasManager::init(const std::wstring& filePath) noexcept {
	filePath_ = filePath.empty() ? getDefaultFilePath() : filePath;
	if (filePath_.empty()) {
		return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
	}

	// Инициализируем контекст для записи (сразу перезаписываем, но пока файл не создаём)
	std::string pathUtf8 = WStringToUTF8(filePath_);
	return writer::openOverwrite(ctx_, pathUtf8);
}

// Загрузка
std::expected<void, std::error_code> AliasManager::load() noexcept {
	if (filePath_.empty()) {
		auto initResult = init(L"");
		if (!initResult) {
			return std::unexpected(initResult.error());
		}
	}

	std::string pathUtf8 = WStringToUTF8(filePath_);
	auto linesResult = writer::readAllLines(pathUtf8);
	if (!linesResult) {
		// Если файл не существует, считаем это успехом (пустой список)
		if (linesResult.error() == std::make_error_code(std::errc::no_such_file_or_directory)) {
			aliases_.clear();
			return {};
		}
		return std::unexpected(linesResult.error());
	}

	aliases_.clear();
	for (const auto& line : *linesResult) {
		auto aliasResult = parseAliasLine(line);
		if (aliasResult) {
			aliases_.push_back(*aliasResult);
		}
		else {
			// Игнорируем неверные строки (можно залогировать)
			std::cerr << "Skipping invalid alias line: " << line << '\n';
		}
	}
	return {};
}

// Сохранение
std::expected<void, std::error_code> AliasManager::save() noexcept {
	if (filePath_.empty()) {
		return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
	}

	// Переоткрываем файл в режиме перезаписи
	std::string pathUtf8 = WStringToUTF8(filePath_);
	auto reopenResult = writer::openOverwrite(ctx_, pathUtf8);
	if (!reopenResult) {
		return std::unexpected(reopenResult.error());
	}

	// Пишем каждую строку
	for (const auto& alias : aliases_) {
		std::string line = serializeAlias(alias);
		auto writeResult = writer::write(ctx_, line, nullptr); // без форматтера
		if (!writeResult) {
			return std::unexpected(writeResult.error());
		}
	}
	return {};
}

// Добавление/обновление
std::expected<void, std::error_code> AliasManager::addOrUpdate(const Alias& alias) noexcept {
	if (!isValidAlias(alias)) {
		return std::unexpected(std::make_error_code(std::errc::invalid_argument));
	}

	for (auto& a : aliases_) {
		if (a.name == alias.name) {
			a.path = alias.path;
			return save();
		}
	}
	aliases_.push_back(alias);
	return save();
}

// Удаление
std::expected<void, std::error_code> AliasManager::remove(const std::wstring& name) noexcept {
	auto it = std::find_if(aliases_.begin(), aliases_.end(),
		[&name](const Alias& a) { return a.name == name; });
	if (it == aliases_.end()) {
		return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
	}
	aliases_.erase(it);
	return save();
}

// Поиск
std::expected<const Alias*, std::error_code> AliasManager::find(const std::wstring& name) const noexcept {
	for (const auto& a : aliases_) {
		if (a.name == name) {
			return &a;
		}
	}
	return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
}

// Очистка
std::expected<void, std::error_code> AliasManager::clear() noexcept {
	aliases_.clear();
	return save();
}