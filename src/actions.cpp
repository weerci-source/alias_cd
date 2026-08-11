#include "actions.h"
#include "utils/utilites.h"
#include "pure.h"
#include <memory>
#include "PanelData.h"

Actions::Result Actions::openAliasesPanel(const PluginContext &ctx) const noexcept
{
	effects_.log("openAliasesPanel");
	auto data = std::make_unique<PanelData>();
	if (!data)
		return std::unexpected(std::make_error_code(std::errc::not_enough_memory));
	data->aliases = storage_.getAll();
	effects_.log("Panel opened, aliases count: " + std::to_string(data->aliases.size()));
	return data.release();
}

Actions::Result Actions::saveAlias(const PluginContext &ctx, std::wstring aliasName) const noexcept
{
	aliasName = trim(aliasName);
	if (aliasName.empty())
		return std::unexpected(std::make_error_code(std::errc::invalid_argument));

	auto currentDir = fs_.getCurrentDir();
	if (!currentDir)
		return std::unexpected(currentDir.error());

	Alias newAlias{aliasName, *currentDir};
	auto saveResult = storage_.addOrUpdate(newAlias);
	if (!saveResult)
		return std::unexpected(saveResult.error());

	effects_.showInfo(ctx, L"Alias \"" + aliasName + L"\" saved as \"" + *currentDir + L"\"");
	return INVALID_HANDLE_VALUE;
}

Actions::Result Actions::gotoAlias(const PluginContext &ctx, std::wstring aliasName) const noexcept
{
	aliasName = trim(aliasName);
	if (aliasName.empty())
		return std::unexpected(std::make_error_code(std::errc::invalid_argument));

	auto found = storage_.find(aliasName);
	if (!found)
		return std::unexpected(found.error());

	auto setDirResult = fs_.setCurrentDir((*found)->path);
	if (!setDirResult)
		return std::unexpected(setDirResult.error());

	auto updateResult = effects_.updateActivePanel(ctx, (*found)->path);
	if (!updateResult)
		return std::unexpected(updateResult.error());

	effects_.showInfo(ctx, L"Changed to \"" + (*found)->path + L"\"");
	return INVALID_HANDLE_VALUE;
}

Actions::Result Actions::processOpenCommand(const PluginContext &ctx, const std::wstring &cmdLine) const noexcept
{
	using namespace pure;

	std::wstring cmd = normalizeCommand(cmdLine);
	if (!isCdCommand(cmd))
		return openAliasesPanel(ctx);

	std::wstring arg = extractArgument(cmd);
	switch (classifyCommand(arg))
	{
	case CommandType::Panel:
		return openAliasesPanel(ctx);
	case CommandType::Save:
		return saveAlias(ctx, arg.substr(1));
	case CommandType::Goto:
		return gotoAlias(ctx, arg);
	default:
		return std::unexpected(std::make_error_code(std::errc::invalid_argument));
	}
}