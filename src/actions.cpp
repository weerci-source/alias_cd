#include "actions.h"
#include "effects.h"
#include "models/alias_manager.h"
#include "utils/utilites.h"
#include "plugin_context.h"
#include <memory>
#include <vector>

// Вспомогательная структура для панели (определена в alias_cd.cpp, но нам нужно её объявление)
struct PanelData {
    std::vector<Alias> aliases;
};

namespace actions {

    Result openAliasesPanel(const PluginContext& ctx) noexcept {
        effects::log("openAliasesPanel");
        auto data = std::make_unique<PanelData>();
        if (!data)
            return std::unexpected(std::make_error_code(std::errc::not_enough_memory));
        data->aliases = AliasManager::Instance().getAll();
        effects::log("Panel opened, aliases count: " + std::to_string(data->aliases.size()));
        return data.release();
    }

    Result saveAlias(const PluginContext& ctx, std::wstring aliasName) noexcept {
        aliasName = trim(aliasName);
        if (aliasName.empty())
            return std::unexpected(std::make_error_code(std::errc::invalid_argument));

        auto currentDir = getCurrentDirW();
        if (!currentDir)
            return std::unexpected(currentDir.error());

        Alias newAlias{ aliasName, *currentDir };
        auto saveResult = AliasManager::Instance().addOrUpdate(newAlias);
        if (!saveResult)
            return std::unexpected(saveResult.error());

        effects::showInfo(ctx, L"Alias \"" + aliasName + L"\" saved as \"" + *currentDir + L"\"");
        return INVALID_HANDLE_VALUE;
    }

    Result gotoAlias(const PluginContext& ctx, std::wstring aliasName) noexcept {
        aliasName = trim(aliasName);
        if (aliasName.empty())
            return std::unexpected(std::make_error_code(std::errc::invalid_argument));

        auto found = AliasManager::Instance().find(aliasName);
        if (!found)
            return std::unexpected(found.error());

        auto setDirResult = setCurrentDirW((*found)->path);
        if (!setDirResult)
            return std::unexpected(setDirResult.error());

        auto updateResult = effects::updateActivePanel(ctx);
        if (!updateResult)
            return std::unexpected(updateResult.error());

        effects::showInfo(ctx, L"Changed to \"" + (*found)->path + L"\"");
        return INVALID_HANDLE_VALUE;
    }

    Result processOpenCommand(const PluginContext& ctx, const std::wstring& cmdLine) noexcept {
        // Здесь используем чистые функции, которые сейчас лежат в alias_cd.cpp.
        // Мы их вынесем позже, поэтому пока оставим заглушку, чтобы скомпилировать.
        // Вызовем openAliasesPanel по умолчанию.
        return openAliasesPanel(ctx);
    }
}