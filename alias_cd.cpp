#include <farplug-wide.h>
#include "src/models/alias.h"
#include "src/models/alias_manager.h"
#include "src/farwrapper/far2l_wrappers.h"
#include "src/utils/utilites.h"
#include <expected>
#include <system_error>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <fstream>
#include <mutex>
#include <optional>
#include <functional>
#include "src/plugin_context.h"
#include "src/effects.h"   
#include "src/actions.h"


// ============================================================
//  Структура данных панели (необходима для GetFindData)
// ============================================================
struct PanelData {
    std::vector<Alias> aliases;
};

static std::optional<PluginContext> g_ctx;   // глобальный контекст (инициализируется один раз)

// ============================================================
//  Вспомогательные чистые функции (без побочных эффектов)
// ============================================================
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

    enum class CommandType { Panel, Save, Goto, Unknown };
    CommandType classifyCommand(const std::wstring& arg) noexcept {
        if (arg.empty()) return CommandType::Panel;
        if (arg[0] == L':') return CommandType::Save;
        return CommandType::Goto;
    }

} // namespace pure


// ============================================================
//  Экспортируемые функции плагина (точки входа FAR)
// ============================================================

SHAREDSYMBOL int WINAPI EXP_NAME(GetMinFarVersion)() {
    return FARMANAGERVERSION;
}

SHAREDSYMBOL void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo* info) {
    if (!info) {
        effects::log("SetStartupInfo: info is null");
        return;
    }

    PluginContext ctx;
    ctx.Info = *info;
    ctx.FSF = info->FSF;
    ctx.Info.FSF = ctx.FSF;

    auto locResult = initLocale();
    if (!locResult) {
        effects::log("Failed to set locale");
        effects::showError(ctx, L"Failed to set locale");
        return;
    }

    auto& mgr = AliasManager::Instance();
    auto initResult = mgr.init(L"");
    if (!initResult) {
        effects::log("Failed to init alias storage");
        effects::showError(ctx, L"Failed to init alias storage");
        return;
    }

    auto loadResult = mgr.load();
    if (!loadResult) {
        effects::log("Failed to load aliases");
        effects::showError(ctx, L"Failed to load aliases");
        return;
    }

    g_ctx.emplace(std::move(ctx));
    effects::log("Plugin initialized successfully (functional style)");
}

SHAREDSYMBOL HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
    effects::log("OpenPlugin called");

    if (!pure::isInitialized()) {
        effects::showError(pure::context(), L"Plugin not initialized");
        return INVALID_HANDLE_VALUE;
    }

    const PluginContext& ctx = pure::context();

    const wchar_t* cmdLine = reinterpret_cast<const wchar_t*>(Item);
    std::wstring cmd = cmdLine ? cmdLine : L"";

    if (OpenFrom == OPEN_PLUGINSMENU || cmd.empty()) {
        auto result = actions::openAliasesPanel(ctx);
        if (!result) {
            effects::showError(ctx, L"Failed to open aliases panel");
            return INVALID_HANDLE_VALUE;
        }
        return *result;
    }

    auto result = actions::processOpenCommand(ctx, cmd);
    if (!result) {
        effects::showError(ctx, L"Command failed");
        return INVALID_HANDLE_VALUE;
    }
    return *result;
}

SHAREDSYMBOL void WINAPI EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
    effects::log("ClosePlugin called");
    if (hPlugin && hPlugin != INVALID_HANDLE_VALUE)
        delete static_cast<PanelData*>(hPlugin);
}

SHAREDSYMBOL void WINAPI EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin, struct OpenPluginInfo* info) {
    if (!info) return;
    info->StructSize = sizeof(*info);
    if (!pure::isInitialized()) return;

    static const wchar_t* title = L"Aliases";
    static const wchar_t* columnTitles[] = { L"Alias", L"Path" };
    static struct PanelMode modes[] = {
        { L"N,C", L"0,0", columnTitles, 0, 0, 0, 0, nullptr, nullptr, {0,0} }
    };

    info->PanelTitle = title;
    info->Format = nullptr;
    info->PanelModesArray = modes;
    info->PanelModesNumber = 1;
    info->StartPanelMode = 0;
    info->InfoLines = nullptr;
    info->InfoLinesNumber = 0;
    info->DescrFiles = nullptr;
    info->DescrFilesNumber = 0;
    info->CurDir = nullptr;
    info->Flags = 0;
    info->HostFile = nullptr;
    info->StartSortMode = 0;
    info->StartSortOrder = 0;
    info->KeyBar = nullptr;
    info->ShortcutData = nullptr;
    info->CurURL = nullptr;
    info->Reserved = 0;
}

SHAREDSYMBOL int WINAPI EXP_NAME(GetFindData)(HANDLE hPlugin, struct PluginPanelItem** pPanelItem,
                                              int* pItemsNumber, int OpMode) {
    effects::log("GetFindData called");
    if (!pPanelItem || !pItemsNumber) {
        effects::log("GetFindData: bad arguments");
        return FALSE;
    }
    *pPanelItem = nullptr;
    *pItemsNumber = 0;

    if (!pure::isInitialized()) {
        effects::log("GetFindData: plugin not initialized");
        return FALSE;
    }

    auto* data = static_cast<PanelData*>(hPlugin);
    if (!data) {
        effects::log("GetFindData: no PanelData");
        return FALSE;
    }

    if (data->aliases.empty()) {
        effects::log("GetFindData: empty aliases list");
        return TRUE;
    }

    const auto& aliases = data->aliases;
    int count = static_cast<int>(aliases.size());
    effects::log("GetFindData: aliases count = " + std::to_string(count));

    auto items = std::make_unique<PluginPanelItem[]>(count);
    if (!items) {
        effects::log("GetFindData: memory allocation failed");
        return FALSE;
    }

    for (int i = 0; i < count; ++i) {
        const auto& a = aliases[i];
        auto nameCopy = std::make_unique<wchar_t[]>(a.name.size() + 1);
        if (!nameCopy) {
            effects::log("GetFindData: name allocation failed");
            return FALSE;
        }
        wcscpy(nameCopy.get(), a.name.c_str());

        auto descCopy = std::make_unique<wchar_t[]>(a.path.size() + 1);
        if (!descCopy) {
            effects::log("GetFindData: desc allocation failed");
            return FALSE;
        }
        wcscpy(descCopy.get(), a.path.c_str());

        items[i].FindData.lpwszFileName = nameCopy.release();
        items[i].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        items[i].FindData.nFileSize = 0;
        items[i].FindData.nPhysicalSize = 0;
        items[i].FindData.ftCreationTime.dwLowDateTime = 0;
        items[i].FindData.ftCreationTime.dwHighDateTime = 0;
        items[i].FindData.ftLastAccessTime.dwLowDateTime = 0;
        items[i].FindData.ftLastAccessTime.dwHighDateTime = 0;
        items[i].FindData.ftLastWriteTime.dwLowDateTime = 0;
        items[i].FindData.ftLastWriteTime.dwHighDateTime = 0;
        items[i].FindData.dwUnixMode = 0;

        items[i].UserData = 0;
        items[i].Flags = 0;
        items[i].NumberOfLinks = 0;
        items[i].Description = descCopy.release();
        items[i].Owner = nullptr;
        items[i].Group = nullptr;
        items[i].CustomColumnData = nullptr;
        items[i].CustomColumnNumber = 0;
        items[i].CRC32 = 0;
        items[i].Reserved[0] = 0;
        items[i].Reserved[1] = 0;
    }

    *pPanelItem = items.release();
    *pItemsNumber = count;
    effects::log("GetFindData: success");
    return TRUE;
}

SHAREDSYMBOL void WINAPI EXP_NAME(FreeFindData)(HANDLE hPlugin, struct PluginPanelItem* pPanelItem,
                                                int pItemsNumber) {
    effects::log("FreeFindData called");
    if (!pPanelItem) return;
    for (int i = 0; i < pItemsNumber; ++i) {
        delete[] pPanelItem[i].FindData.lpwszFileName;
        delete[] pPanelItem[i].Description;
    }
    delete[] pPanelItem;
}

SHAREDSYMBOL int WINAPI EXP_NAME(SetDirectory)(HANDLE hPlugin, const wchar_t* Dir, int OpMode) {
    effects::log("SetDirectory called");
    if (!pure::isInitialized()) return FALSE;

    const PluginContext& ctx = pure::context();
    auto* data = static_cast<PanelData*>(hPlugin);
    if (!data) return FALSE;

    std::wstring dir = Dir ? Dir : L"";
    dir = trim(dir);
    if (dir.empty()) return FALSE;

    auto it = std::find_if(data->aliases.begin(), data->aliases.end(),
                           [&dir](const Alias& a) { return a.name == dir; });
    if (it == data->aliases.end()) {
        effects::showError(ctx, L"Alias \"" + dir + L"\" not found");
        return FALSE;
    }

    auto setRes = setCurrentDirW(it->path);
    if (!setRes) {
        effects::showError(ctx, L"Cannot change to \"" + it->path + L"\"");
        return FALSE;
    }

    auto updateRes = effects::updateActivePanel(ctx);
    if (!updateRes) {
        effects::showError(ctx, L"Directory changed, but panel update failed");
    }

    auto closeRes = effects::closePlugin(ctx, hPlugin);
    return TRUE;
}

SHAREDSYMBOL void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo* info) {
    if (!info) return;
    info->StructSize = sizeof(*info);
    info->Flags = PF_FULLCMDLINE;
    static const wchar_t* menuStrings[] = { L"Alias CD" };
    info->PluginMenuStrings = menuStrings;
    info->PluginMenuStringsNumber = 1;
    info->CommandPrefix = L"cd";
    info->DiskMenuStrings = nullptr;
    info->DiskMenuStringsNumber = 0;
    info->PluginConfigStrings = nullptr;
    info->PluginConfigStringsNumber = 0;
    info->SysID = 0;
}