#include <farplug-wide.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>
#include <climits>
#include <vector>

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

#define MAX_ALIASES 100
#define MAX_PATH_LEN 4096

struct Alias
{
    wchar_t name[256];
    wchar_t path[MAX_PATH_LEN];
};

static Alias g_aliases[MAX_ALIASES];
static int g_aliasCount = 0;
static bool g_aliasesLoaded = false;

struct AliasPanelState
{
    std::vector<Alias> aliases;
};

void Log(const char *msg)
{
    FILE *f = fopen("/tmp/alias_cd.log", "a");
    if (f)
    {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

void LogW(const wchar_t *msg)
{
    char mb[512];
    wcstombs(mb, msg, sizeof(mb));
    Log(mb);
}

void SetWideString(wchar_t *dst, size_t dstSize, const wchar_t *src)
{
    if (!dst || dstSize == 0)
    {
        return;
    }
    if (!src)
    {
        dst[0] = L'\0';
        return;
    }

    size_t copySize = wcslen(src);
    if (copySize >= dstSize)
    {
        copySize = dstSize - 1;
    }

    wcsncpy(dst, src, copySize);
    dst[copySize] = L'\0';
}

std::string WideToUtf8(const wchar_t *input)
{
    std::string result;
    if (!input)
    {
        return result;
    }

    std::vector<char> buffer(wcslen(input) * 4 + 1);
    size_t converted = wcstombs(buffer.data(), input, buffer.size());
    if (converted == (size_t)-1)
    {
        return result;
    }

    result.assign(buffer.data(), converted);
    return result;
}

std::wstring Utf8ToWide(const std::string &input)
{
    std::wstring result;
    if (input.empty())
    {
        return result;
    }

    std::vector<wchar_t> buffer(input.size() + 1);
    size_t converted = mbstowcs(buffer.data(), input.c_str(), buffer.size());
    if (converted == (size_t)-1)
    {
        return result;
    }

    result.assign(buffer.data(), converted);
    return result;
}

std::filesystem::path GetStorageFilePath()
{
    const char *home = std::getenv("HOME");
    std::filesystem::path base = home ? std::filesystem::path(home) : std::filesystem::current_path();
    std::filesystem::path configDir = base / ".config" / "far2l";
    std::error_code ec;
    std::filesystem::create_directories(configDir, ec);
    return configDir / "alias_cd_aliases.txt";
}

bool LoadAliasesFromStorage()
{
    g_aliasCount = 0;

    std::filesystem::path storageFile = GetStorageFilePath();
    std::ifstream file(storageFile.c_str(), std::ios::in);
    if (!file)
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        size_t sep = line.find('\t');
        if (sep == std::string::npos)
        {
            continue;
        }

        std::wstring name = Utf8ToWide(line.substr(0, sep));
        std::wstring path = Utf8ToWide(line.substr(sep + 1));
        if (name.empty() || path.empty())
        {
            continue;
        }

        if (g_aliasCount >= MAX_ALIASES)
        {
            break;
        }

        SetWideString(g_aliases[g_aliasCount].name, sizeof(g_aliases[g_aliasCount].name) / sizeof(g_aliases[g_aliasCount].name[0]), name.c_str());
        SetWideString(g_aliases[g_aliasCount].path, sizeof(g_aliases[g_aliasCount].path) / sizeof(g_aliases[g_aliasCount].path[0]), path.c_str());
        ++g_aliasCount;
    }

    return true;
}

bool SaveAliasesToStorage()
{
    std::filesystem::path storageFile = GetStorageFilePath();
    std::ofstream file(storageFile.c_str(), std::ios::out | std::ios::trunc);
    if (!file)
    {
        return false;
    }

    for (int i = 0; i < g_aliasCount; ++i)
    {
        std::string nameBytes = WideToUtf8(g_aliases[i].name);
        std::string pathBytes = WideToUtf8(g_aliases[i].path);
        if (nameBytes.empty() || pathBytes.empty())
        {
            continue;
        }

        file << nameBytes << '\t' << pathBytes << '\n';
    }

    return true;
}

wchar_t *DuplicateWideString(const wchar_t *value)
{
    if (!value)
    {
        return nullptr;
    }

    size_t length = wcslen(value) + 1;
    wchar_t *copy = new wchar_t[length];
    wcscpy(copy, value);
    return copy;
}

AliasPanelState *CreateAliasPanelState()
{
    auto *state = new AliasPanelState();
    state->aliases.reserve(g_aliasCount);
    for (int i = 0; i < g_aliasCount; ++i)
    {
        state->aliases.push_back(g_aliases[i]);
    }
    return state;
}

const wchar_t *GetCurrentDir()
{
    static wchar_t wbuf[MAX_PATH_LEN];
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)))
    {
        size_t len = mbstowcs(wbuf, buf, sizeof(wbuf) / sizeof(wchar_t));
        if (len != (size_t)-1)
        {
            return wbuf;
        }
    }
    return L"";
}

int SetCurrentDir(const wchar_t *path)
{
    char mbpath[PATH_MAX];
    if (wcstombs(mbpath, path, sizeof(mbpath)) == (size_t)-1)
    {
        Log("wcstombs failed");
        return 0;
    }
    if (chdir(mbpath) != 0)
    {
        Log("chdir failed");
        return 0;
    }
    Log("chdir succeeded, now updating panel via Control");

    HANDLE hPanel = PANEL_ACTIVE; // (HANDLE)-1

    // Меняем директорию панели
    if (Info.Control(hPanel, FCTL_SETPANELDIR, 0, (LONG_PTR)path))
    {
        Log("FCTL_SETPANELDIR succeeded");
    }
    else
    {
        Log("FCTL_SETPANELDIR failed");
    }

    // Принудительно обновляем панель (перечитываем содержимое)
    if (Info.Control(hPanel, FCTL_UPDATEPANEL, 0, 0))
    {
        Log("FCTL_UPDATEPANEL succeeded");
    }
    else
    {
        Log("FCTL_UPDATEPANEL failed");
    }

    // Перерисовываем панель
    if (Info.Control(hPanel, FCTL_REDRAWPANEL, 0, 0))
    {
        Log("FCTL_REDRAWPANEL succeeded");
    }
    else
    {
        Log("FCTL_REDRAWPANEL failed");
    }

    return 1;
}

int FindAlias(const wchar_t *name)
{
    for (int i = 0; i < g_aliasCount; ++i)
    {
        if (wcscmp(g_aliases[i].name, name) == 0)
            return i;
    }
    return -1;
}

SHAREDSYMBOL int WINAPI EXP_NAME(GetMinFarVersion)()
{
    return FARMANAGERVERSION;
}

SHAREDSYMBOL void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *info)
{
    Info = *info;
    FSF = *info->FSF;
    Info.FSF = &FSF;
    if (!g_aliasesLoaded)
    {
        LoadAliasesFromStorage();
        g_aliasesLoaded = true;
    }
    Log("SetStartupInfo called");
}

SHAREDSYMBOL HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item)
{
    if (!g_aliasesLoaded)
    {
        LoadAliasesFromStorage();
        g_aliasesLoaded = true;
    }
    Log("OpenPlugin called");

    if (OpenFrom == OPEN_PLUGINSMENU)
    {
        return reinterpret_cast<HANDLE>(CreateAliasPanelState());
    }

    const wchar_t *cmdLine = reinterpret_cast<const wchar_t *>(Item);
    if (cmdLine && *cmdLine)
    {
        Log("Command line not empty");

        // Удаляем префикс "cd:" если он есть
        if (wcsncmp(cmdLine, L"cd:", 3) == 0)
        {
            cmdLine += 3;
        }
        // Теперь cmdLine указывает на часть после "cd:"
        // (например, ":test" или "test")

        wchar_t cmd[512];
        wcsncpy(cmd, cmdLine, 511);
        cmd[511] = 0;
        LogW(cmd);

        if (cmd[0] == L':')
        {
            Log("Save command");
            wchar_t *alias = cmd + 1;
            if (*alias == 0)
            {
                Log("Empty alias");
                return INVALID_HANDLE_VALUE;
            }
            const wchar_t *path = GetCurrentDir();
            if (!path || !*path)
            {
                Log("Failed to get current dir");
                return INVALID_HANDLE_VALUE;
            }
            int idx = FindAlias(alias);
            if (idx == -1)
            {
                if (g_aliasCount < MAX_ALIASES)
                {
                    idx = g_aliasCount++;
                    SetWideString(g_aliases[idx].name, sizeof(g_aliases[idx].name) / sizeof(g_aliases[idx].name[0]), alias);
                    SetWideString(g_aliases[idx].path, sizeof(g_aliases[idx].path) / sizeof(g_aliases[idx].path[0]), path);
                    if (SaveAliasesToStorage())
                    {
                        Log("Alias saved");
                    }
                    else
                    {
                        Log("Failed to save aliases to storage");
                    }
                }
                else
                {
                    Log("Too many aliases");
                }
            }
            else
            {
                SetWideString(g_aliases[idx].path, sizeof(g_aliases[idx].path) / sizeof(g_aliases[idx].path[0]), path);
                if (SaveAliasesToStorage())
                {
                    Log("Alias updated");
                }
                else
                {
                    Log("Failed to save aliases to storage");
                }
            }
            return INVALID_HANDLE_VALUE;
        }
        else if (cmd[0] == L'\0')
        {
            return reinterpret_cast<HANDLE>(CreateAliasPanelState());
        }
        else
        {
            Log("Goto command");
            wchar_t *alias = cmd;
            int idx = FindAlias(alias);
            if (idx == -1)
            {
                Log("Alias not found");
                return INVALID_HANDLE_VALUE;
            }
            if (SetCurrentDir(g_aliases[idx].path))
            {
                Log("Directory changed");
            }
            else
            {
                Log("Failed to change directory");
            }
            return INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        return reinterpret_cast<HANDLE>(CreateAliasPanelState());
    }

    return INVALID_HANDLE_VALUE;
}

SHAREDSYMBOL int WINAPI EXP_NAME(GetFindData)(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{
    if (!pPanelItem || !pItemsNumber)
    {
        return FALSE;
    }

    auto *state = reinterpret_cast<AliasPanelState *>(hPlugin);
    if (!state)
    {
        *pPanelItem = nullptr;
        *pItemsNumber = 0;
        return FALSE;
    }

    const int count = static_cast<int>(state->aliases.size());
    auto *items = new PluginPanelItem[count];
    memset(items, 0, sizeof(PluginPanelItem) * count);

    for (int i = 0; i < count; ++i)
    {
        items[i].FindData.lpwszFileName = DuplicateWideString(state->aliases[i].name);
        items[i].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        items[i].Description = DuplicateWideString(state->aliases[i].path);
        items[i].UserData = i;
    }

    *pPanelItem = items;
    *pItemsNumber = count;
    return TRUE;
}

SHAREDSYMBOL void WINAPI EXP_NAME(FreeFindData)(HANDLE hPlugin, struct PluginPanelItem *panelItem, int itemsNumber)
{
    if (!panelItem)
    {
        return;
    }

    for (int i = 0; i < itemsNumber; ++i)
    {
        delete[] panelItem[i].FindData.lpwszFileName;
        delete[] panelItem[i].Description;
    }

    delete[] panelItem;
}

SHAREDSYMBOL void WINAPI EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin, struct OpenPluginInfo *info)
{
    if (!info)
    {
        return;
    }

    info->StructSize = sizeof(*info);
    info->Flags = OPIF_USEFILTER | OPIF_USESORTGROUPS | OPIF_USEHIGHLIGHTING | OPIF_ADDDOTS;
    info->PanelTitle = L"Alias CD";
    info->HostFile = nullptr;
    info->CurDir = nullptr;
}

SHAREDSYMBOL int WINAPI EXP_NAME(ProcessHostFile)(HANDLE hPlugin, struct PluginPanelItem *panelItem, int itemsNumber, int OpMode)
{
    if (!panelItem || itemsNumber <= 0)
    {
        return FALSE;
    }

    auto *state = reinterpret_cast<AliasPanelState *>(hPlugin);
    if (!state)
    {
        return FALSE;
    }

    const int index = static_cast<int>(panelItem[0].UserData);
    if (index < 0 || index >= static_cast<int>(state->aliases.size()))
    {
        return FALSE;
    }

    const wchar_t *path = state->aliases[index].path;
    if (SetCurrentDir(path))
    {
        Info.Control(hPlugin, FCTL_CLOSEPLUGIN, 0, 0);
        return TRUE;
    }

    return FALSE;
}

SHAREDSYMBOL void WINAPI EXP_NAME(ClosePlugin)(HANDLE hPlugin)
{
    delete reinterpret_cast<AliasPanelState *>(hPlugin);
}

SHAREDSYMBOL void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *info)
{
    info->StructSize = sizeof(*info);
    info->Flags = PF_FULLCMDLINE;
    static const wchar_t *menuStrings[] = {L"Alias CD"};
    info->PluginMenuStrings = menuStrings;
    info->PluginMenuStringsNumber = 1;
    info->CommandPrefix = L"cd";
}

SHAREDSYMBOL int WINAPI EXP_NAME(SetDirectory)(HANDLE hPlugin, const wchar_t *Dir, int OpMode)
{
    // Log((" SetDirectory: " + WideToUtf8(Dir)).c_str());
    if (!hPlugin || !Dir)
    {
        return FALSE;
    }

    int idx = FindAlias(Dir);
    auto a = g_aliases[idx].path;

    Log((" SetDirectory: " + WideToUtf8(Dir) + WideToUtf8(a)).c_str());


    if (SetCurrentDir(g_aliases[idx].path))
    {
        Info.Control(hPlugin, FCTL_CLOSEPLUGIN, 0, 0);
        return TRUE;
    }
}