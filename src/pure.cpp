#include "pure.h"
#include "utils/utilites.h"
#include <algorithm>
#include <optional>

static std::optional<PluginContext> g_ctx;

namespace pure {

	bool isInitialized() noexcept {
		return g_ctx.has_value();
	}

	const PluginContext& context() noexcept {
		return *g_ctx;
	}

	std::wstring normalizeCommand(std::wstring cmd) {
		std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::towlower);
		return trim(cmd);
	}

	bool isCdCommand(const std::wstring& cmd) noexcept {
		return cmd.find(L"cd:") == 0;
	}

	std::wstring extractArgument(const std::wstring& cmd) noexcept {
		if (cmd.size() < 3) return L"";
		return trim(cmd.substr(3));
	}

	CommandType classifyCommand(const std::wstring& arg) noexcept {
		if (arg.empty()) return CommandType::Panel;
		if (arg[0] == L':') return CommandType::Save;
		return CommandType::Goto;
	}

	void setContext(PluginContext&& ctx) noexcept {
		g_ctx.emplace(std::move(ctx));
	}
} // namespace pure