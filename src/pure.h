#pragma once

#include <string>
#include "plugin_context.h"

namespace pure
{

	bool isInitialized() noexcept;
	const PluginContext &context() noexcept;
	void setContext(PluginContext &&ctx) noexcept;

	std::wstring normalizeCommand(std::wstring cmd);
	bool isCdCommand(const std::wstring &cmd) noexcept;
	std::wstring extractArgument(const std::wstring &cmd) noexcept;

	enum class CommandType
	{
		Panel,
		Save,
		Goto,
		Unknown
	};
	CommandType classifyCommand(const std::wstring &arg) noexcept;

}