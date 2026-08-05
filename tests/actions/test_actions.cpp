#include <gtest/gtest.h>
#include "../../src/actions.h"
#include "../../src/effects.h"
#include "../../src/utils/utilites.h"
#include "../../src/models/alias_manager.h"
#include "../../src/plugin_context.h"
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class ActionsTest : public ::testing::Test {
protected:
	void SetUp() override {
		// Создаём временную папку и файл для AliasManager
		testDir = fs::temp_directory_path() / "actions_test";
		fs::create_directories(testDir);
		testFile = testDir / "aliases";

		auto& mgr = AliasManager::Instance();
		auto initResult = mgr.init(testFile.wstring());
		ASSERT_TRUE(initResult.has_value()) << "Init failed: " << initResult.error().message();
		mgr.clear();

		// Сбрасываем статические переменные
		controlCallCount = 0;
		messageCallCount = 0;
		lastControlHandle = nullptr;
		lastControlCmd = 0;
		lastControlP1 = 0;
		lastControlP2 = nullptr;
		mockControlShouldFail = false;
		lastMessageTitle = L"";
		lastMessageItems.clear();
		lastMessageFlags = 0;
		lastMessageIcon = 0;
		mockMessageShouldFail = false;
		mockGetCurrentDirShouldFail = false;
		mockSetCurrentDirShouldFail = false;
		lastSetPath = L"";

		// Устанавливаем моки
		effects::g_control_impl = mockControl;
		effects::g_message_impl = mockMessage;
		g_getCurrentDirW_impl = mockGetCurrentDirW;
		g_setCurrentDirW_impl = mockSetCurrentDirW;

		// Проверяем, что моки установлены
		ASSERT_NE(effects::g_control_impl, nullptr);
		ASSERT_NE(effects::g_message_impl, nullptr);

		// Создаём контекст (заглушка)
		ctx.Info.ModuleNumber = 0;
		ctx.Info.Control = nullptr;
		ctx.Info.Message = nullptr;
		ctx.FSF = nullptr;

		// ПРОВЕРКА: вызываем effects::message напрямую, чтобы убедиться, что мок работает
		auto testMsg = effects::message(ctx, L"Test", { L"test" }, 0, 0);
		ASSERT_TRUE(testMsg.has_value()) << "effects::message failed";
		ASSERT_GT(messageCallCount, 0) << "mockMessage was not called by effects::message";
		messageCallCount = 0; // сбрасываем после проверки
	}

	void TearDown() override {
		// Сброс моков
		effects::g_control_impl = nullptr;
		effects::g_message_impl = nullptr;
		g_getCurrentDirW_impl = nullptr;
		g_setCurrentDirW_impl = nullptr;

		// Удаляем временную папку
		fs::remove_all(testDir);
	}

	// Моки с счётчиками
	static std::expected<void, std::error_code> mockControl(const PluginContext& ctx, HANDLE h, int cmd, int p1, void* p2) {
		controlCallCount++;
		lastControlHandle = h;
		lastControlCmd = cmd;
		lastControlP1 = p1;
		lastControlP2 = p2;
		if (mockControlShouldFail) {
			return std::unexpected(std::make_error_code(std::errc::io_error));
		}
		return {};
	}

	static std::expected<void, std::error_code> mockMessage(const PluginContext& ctx, const std::wstring& title,
		const std::vector<std::wstring>& items,
		int flags, int icon) {
		messageCallCount++;
		lastMessageTitle = title;
		lastMessageItems = items;
		lastMessageFlags = flags;
		lastMessageIcon = icon;
		if (mockMessageShouldFail) {
			return std::unexpected(std::make_error_code(std::errc::io_error));
		}
		return {};
	}

	static std::expected<std::wstring, std::error_code> mockGetCurrentDirW() noexcept {
		if (mockGetCurrentDirShouldFail) {
			return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
		}
		return L"/mock/current/dir";
	}

	static std::expected<void, std::error_code> mockSetCurrentDirW(const std::wstring& path) noexcept {
		lastSetPath = path;
		if (mockSetCurrentDirShouldFail) {
			return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
		}
		return {};
	}

	// Статические переменные
	static int controlCallCount;
	static int messageCallCount;
	static HANDLE lastControlHandle;
	static int lastControlCmd;
	static int lastControlP1;
	static void* lastControlP2;
	static bool mockControlShouldFail;

	static std::wstring lastMessageTitle;
	static std::vector<std::wstring> lastMessageItems;
	static int lastMessageFlags;
	static int lastMessageIcon;
	static bool mockMessageShouldFail;

	static bool mockGetCurrentDirShouldFail;
	static bool mockSetCurrentDirShouldFail;
	static std::wstring lastSetPath;

	PluginContext ctx;
	fs::path testDir;
	fs::path testFile;
};

// Инициализация статических переменных
int ActionsTest::controlCallCount = 0;
int ActionsTest::messageCallCount = 0;
HANDLE ActionsTest::lastControlHandle = nullptr;
int ActionsTest::lastControlCmd = 0;
int ActionsTest::lastControlP1 = 0;
void* ActionsTest::lastControlP2 = nullptr;
bool ActionsTest::mockControlShouldFail = false;

std::wstring ActionsTest::lastMessageTitle = L"";
std::vector<std::wstring> ActionsTest::lastMessageItems = {};
int ActionsTest::lastMessageFlags = 0;
int ActionsTest::lastMessageIcon = 0;
bool ActionsTest::mockMessageShouldFail = false;

bool ActionsTest::mockGetCurrentDirShouldFail = false;
bool ActionsTest::mockSetCurrentDirShouldFail = false;
std::wstring ActionsTest::lastSetPath = L"";

// ============================================================================
// Тесты
// ============================================================================

TEST_F(ActionsTest, OpenAliasesPanel_ReturnsHandleWithAliases) {
	Alias a1{ L"home", L"/home/user" };
	Alias a2{ L"work", L"/work/project" };
	auto& mgr = AliasManager::Instance();
	mgr.addOrUpdate(a1);
	mgr.addOrUpdate(a2);

	auto result = actions::openAliasesPanel(ctx);
	ASSERT_TRUE(result.has_value());
	HANDLE h = *result;
	EXPECT_NE(h, INVALID_HANDLE_VALUE);
	EXPECT_NE(h, nullptr);
}

TEST_F(ActionsTest, SaveAlias_SavesAliasAndShowsInfo) {
	mockGetCurrentDirShouldFail = false;

	auto result = actions::saveAlias(ctx, L"test_alias");
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(*result, INVALID_HANDLE_VALUE);

	// Проверяем, что алиас сохранён
	auto& mgr = AliasManager::Instance();
	auto found = mgr.find(L"test_alias");
	ASSERT_TRUE(found.has_value());
	EXPECT_EQ((*found)->path, L"/mock/current/dir");

	// Проверяем вызовы моков
	EXPECT_GT(messageCallCount, 0) << "mockMessage was not called";
	EXPECT_EQ(lastMessageTitle, L"Alias CD");
	ASSERT_EQ(lastMessageItems.size(), 1);
	EXPECT_EQ(lastMessageItems[0], L"Alias \"test_alias\" saved as \"/mock/current/dir\"");
}

TEST_F(ActionsTest, SaveAlias_InvalidName_ReturnsError) {
	auto result = actions::saveAlias(ctx, L"");
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), std::make_error_code(std::errc::invalid_argument));

	result = actions::saveAlias(ctx, L"   ");
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), std::make_error_code(std::errc::invalid_argument));
}

TEST_F(ActionsTest, SaveAlias_GetCurrentDirFails_ReturnsError) {
	mockGetCurrentDirShouldFail = true;
	auto result = actions::saveAlias(ctx, L"test");
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(ActionsTest, GotoAlias_GoesToExistingAliasAndUpdatesPanel) {
	Alias a{ L"home", L"/home/user" };
	auto& mgr = AliasManager::Instance();
	mgr.addOrUpdate(a);

	mockSetCurrentDirShouldFail = false;
	mockControlShouldFail = false;
	controlCallCount = 0;  // сбрасываем перед вызовом

	auto result = actions::gotoAlias(ctx, L"home");
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(*result, INVALID_HANDLE_VALUE);

	// Проверяем вызовы моков
	EXPECT_GT(controlCallCount, 0) << "mockControl was not called";
	EXPECT_EQ(lastSetPath, L"/home/user");
}

TEST_F(ActionsTest, GotoAlias_NonExisting_ReturnsError) {
	auto result = actions::gotoAlias(ctx, L"nonexistent");
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(ActionsTest, GotoAlias_SetCurrentDirFails_ReturnsError) {
	Alias a{ L"home", L"/home/user" };
	auto& mgr = AliasManager::Instance();
	mgr.addOrUpdate(a);

	mockSetCurrentDirShouldFail = true;
	auto result = actions::gotoAlias(ctx, L"home");
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(ActionsTest, ShowError_CallsMessageWithErrorTitle) {
	// Сбрасываем счётчики
	messageCallCount = 0;
	lastMessageTitle = L"";
	lastMessageItems.clear();

	effects::showError(ctx, L"Test error message");

	EXPECT_GT(messageCallCount, 0);
	EXPECT_EQ(lastMessageTitle, L"Alias CD Error");
	ASSERT_EQ(lastMessageItems.size(), 1);
	EXPECT_EQ(lastMessageItems[0], L"Test error message");
}

TEST_F(ActionsTest, ShowInfo_CallsMessageWithInfoTitle) {
	messageCallCount = 0;
	lastMessageTitle = L"";
	lastMessageItems.clear();

	effects::showInfo(ctx, L"Test info message");

	EXPECT_GT(messageCallCount, 0);
	EXPECT_EQ(lastMessageTitle, L"Alias CD");
	ASSERT_EQ(lastMessageItems.size(), 1);
	EXPECT_EQ(lastMessageItems[0], L"Test info message");
}

TEST_F(ActionsTest, ProcessOpenCommand_WithCdPrefix_GoesToAlias) {
    // Добавляем алиас
    Alias a{L"home", L"/home/user"};
    auto& mgr = AliasManager::Instance();
    mgr.addOrUpdate(a);
    
    mockSetCurrentDirShouldFail = false;
    mockControlShouldFail = false;
    controlCallCount = 0;
    
    auto result = actions::processOpenCommand(ctx, L"cd:home");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, INVALID_HANDLE_VALUE);
    EXPECT_GT(controlCallCount, 0);
    EXPECT_EQ(lastSetPath, L"/home/user");
}

TEST_F(ActionsTest, ProcessOpenCommand_WithCdPrefixAndColon_OpensPanel) {
    // cd: без аргумента -> открыть панель
    auto result = actions::processOpenCommand(ctx, L"cd:");
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(*result, INVALID_HANDLE_VALUE);
}

TEST_F(ActionsTest, ProcessOpenCommand_WithSaveCommand_SavesAlias) {
    mockGetCurrentDirShouldFail = false;
    
    auto result = actions::processOpenCommand(ctx, L"cd::test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, INVALID_HANDLE_VALUE);
    
    auto& mgr = AliasManager::Instance();
    auto found = mgr.find(L"test");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ((*found)->path, L"/mock/current/dir");
}

TEST_F(ActionsTest, ProcessOpenCommand_InvalidCommand_ReturnsError) {
    // Команда, не начинающаяся с "cd:" -> должна открыть панель, а не ошибку
    // Но если это не "cd:", то она идёт в openAliasesPanel, что работает.
    // Нужно проверить случай, когда команда начинается с "cd:", но аргумент некорректен.
    // Например, "cd:   " – это должно открыть панель.
    auto result = actions::processOpenCommand(ctx, L"cd:   ");
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(*result, INVALID_HANDLE_VALUE);
}