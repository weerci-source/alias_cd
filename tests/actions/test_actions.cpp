#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "actions.h"
#include "effects.h"
#include "interfaces/IFarApi.h"
#include "interfaces/IFileSystem.h"
#include "interfaces/IAliasStorage.h"
#include "models/alias.h"
#include <memory>
#include <vector>

using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::ReturnRef;
using ::testing::Eq;

// ============================================================================
// Моки интерфейсов
// ============================================================================

class MockFarApi : public IFarApi {
public:
    MOCK_METHOD((std::expected<void, std::error_code>), control, (HANDLE, int, int, void*), (noexcept, override));
    MOCK_METHOD((std::expected<void, std::error_code>), message, (const std::wstring&, const std::vector<std::wstring>&, int, int), (noexcept, override));
};

class MockFileSystem : public IFileSystem {
public:
    MOCK_METHOD((std::expected<std::wstring, std::error_code>), getCurrentDir, (), (noexcept, override));
    MOCK_METHOD((std::expected<void, std::error_code>), setCurrentDir, (const std::wstring&), (noexcept, override));
};

class MockAliasStorage : public IAliasStorage {
public:
    MOCK_METHOD((std::expected<void, std::error_code>), init, (const std::wstring&), (noexcept, override));
    MOCK_METHOD((std::expected<void, std::error_code>), load, (), (noexcept, override));
    MOCK_METHOD((std::expected<void, std::error_code>), save, (), (noexcept, override));
    MOCK_METHOD((std::expected<void, std::error_code>), addOrUpdate, (const Alias&), (noexcept, override));
    MOCK_METHOD((std::expected<void, std::error_code>), remove, (const std::wstring&), (noexcept, override));
    MOCK_METHOD((std::expected<const Alias*, std::error_code>), find, (const std::wstring&), (const, noexcept, override));
    MOCK_METHOD(const std::vector<Alias>&, getAll, (), (const, noexcept, override));
    MOCK_METHOD((std::expected<void, std::error_code>), clear, (), (noexcept, override));
};

class ActionsTest : public ::testing::Test {
protected:
    PluginContext ctx;
};

// ============================================================================
// Тесты
// ============================================================================

TEST_F(ActionsTest, OpenAliasesPanel_ReturnsHandleWithAliases) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    std::vector<Alias> aliases = {
        {L"home", L"/home/user"},
        {L"work", L"/work/project"}
    };
    EXPECT_CALL(mockStorage, getAll())
        .WillOnce(ReturnRef(aliases));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.openAliasesPanel(ctx);
    ASSERT_TRUE(result.has_value());
    HANDLE h = *result;
    EXPECT_NE(h, INVALID_HANDLE_VALUE);
    EXPECT_NE(h, nullptr);
}

TEST_F(ActionsTest, SaveAlias_SavesAliasAndShowsInfo) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    EXPECT_CALL(mockFs, getCurrentDir())
        .WillOnce(Return(std::expected<std::wstring, std::error_code>(L"/mock/current/dir")));

    EXPECT_CALL(mockStorage, addOrUpdate(Eq(Alias{L"test_alias", L"/mock/current/dir"})))
        .WillOnce(Return(std::expected<void, std::error_code>{}));

    EXPECT_CALL(mockFar, message(Eq(L"Alias CD"), _, _, _))
        .WillOnce(Return(std::expected<void, std::error_code>{}));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.saveAlias(ctx, L"test_alias");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, INVALID_HANDLE_VALUE);
}

TEST_F(ActionsTest, SaveAlias_InvalidName_ReturnsError) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.saveAlias(ctx, L"");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::invalid_argument));

    result = actions.saveAlias(ctx, L"   ");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::invalid_argument));
}

TEST_F(ActionsTest, SaveAlias_GetCurrentDirFails_ReturnsError) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    EXPECT_CALL(mockFs, getCurrentDir())
        .WillOnce(Return(std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory))));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.saveAlias(ctx, L"test");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(ActionsTest, GotoAlias_GoesToExistingAliasAndUpdatesPanel) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    Alias existing{L"home", L"/home/user"};
    EXPECT_CALL(mockStorage, find(Eq(L"home")))
        .WillOnce(Return(std::expected<const Alias*, std::error_code>(&existing)));

    EXPECT_CALL(mockFs, setCurrentDir(Eq(L"/home/user")))
        .WillOnce(Return(std::expected<void, std::error_code>{}));

    EXPECT_CALL(mockFar, control(_, _, _, _))
        .Times(3)
        .WillRepeatedly(Return(std::expected<void, std::error_code>{}));

    EXPECT_CALL(mockFar, message(_, _, _, _))
        .WillOnce(Return(std::expected<void, std::error_code>{}));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.gotoAlias(ctx, L"home");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, INVALID_HANDLE_VALUE);
}

TEST_F(ActionsTest, GotoAlias_NonExisting_ReturnsError) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    EXPECT_CALL(mockStorage, find(Eq(L"nonexistent")))
        .WillOnce(Return(std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory))));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.gotoAlias(ctx, L"nonexistent");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(ActionsTest, GotoAlias_SetCurrentDirFails_ReturnsError) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    Alias existing{L"home", L"/home/user"};
    EXPECT_CALL(mockStorage, find(Eq(L"home")))
        .WillOnce(Return(std::expected<const Alias*, std::error_code>(&existing)));

    EXPECT_CALL(mockFs, setCurrentDir(Eq(L"/home/user")))
        .WillOnce(Return(std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory))));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.gotoAlias(ctx, L"home");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(ActionsTest, ShowError_CallsMessageWithErrorTitle) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    std::wstring capturedTitle;
    std::vector<std::wstring> capturedItems;

    EXPECT_CALL(mockFar, message(_, _, _, _))
        .WillOnce(DoAll(
            SaveArg<0>(&capturedTitle),
            SaveArg<1>(&capturedItems),
            Return(std::expected<void, std::error_code>{})
        ));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    effects.showError(ctx, L"Test error message");

    EXPECT_EQ(capturedTitle, L"Alias CD Error");
    ASSERT_EQ(capturedItems.size(), 1);
    EXPECT_EQ(capturedItems[0], L"Test error message");
}

TEST_F(ActionsTest, ShowInfo_CallsMessageWithInfoTitle) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    std::wstring capturedTitle;
    std::vector<std::wstring> capturedItems;

    EXPECT_CALL(mockFar, message(_, _, _, _))
        .WillOnce(DoAll(
            SaveArg<0>(&capturedTitle),
            SaveArg<1>(&capturedItems),
            Return(std::expected<void, std::error_code>{})
        ));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    effects.showInfo(ctx, L"Test info message");

    EXPECT_EQ(capturedTitle, L"Alias CD");
    ASSERT_EQ(capturedItems.size(), 1);
    EXPECT_EQ(capturedItems[0], L"Test info message");
}

TEST_F(ActionsTest, ProcessOpenCommand_WithCdPrefix_GoesToAlias) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    Alias existing{L"home", L"/home/user"};
    EXPECT_CALL(mockStorage, find(Eq(L"home")))
        .WillOnce(Return(std::expected<const Alias*, std::error_code>(&existing)));
    EXPECT_CALL(mockFs, setCurrentDir(Eq(L"/home/user")))
        .WillOnce(Return(std::expected<void, std::error_code>{}));
    EXPECT_CALL(mockFar, control(_, _, _, _))
        .Times(3)
        .WillRepeatedly(Return(std::expected<void, std::error_code>{}));
    EXPECT_CALL(mockFar, message(_, _, _, _))
        .WillOnce(Return(std::expected<void, std::error_code>{}));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.processOpenCommand(ctx, L"cd:home");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, INVALID_HANDLE_VALUE);
}

TEST_F(ActionsTest, ProcessOpenCommand_WithCdPrefixAndColon_OpensPanel) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    std::vector<Alias> empty;
    EXPECT_CALL(mockStorage, getAll())
        .WillOnce(ReturnRef(empty));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.processOpenCommand(ctx, L"cd:");
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(*result, INVALID_HANDLE_VALUE);
}

TEST_F(ActionsTest, ProcessOpenCommand_WithSaveCommand_SavesAlias) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    EXPECT_CALL(mockFs, getCurrentDir())
        .WillOnce(Return(std::expected<std::wstring, std::error_code>(L"/mock/current/dir")));
    EXPECT_CALL(mockStorage, addOrUpdate(Eq(Alias{L"test", L"/mock/current/dir"})))
        .WillOnce(Return(std::expected<void, std::error_code>{}));
    EXPECT_CALL(mockFar, message(_, _, _, _))
        .WillOnce(Return(std::expected<void, std::error_code>{}));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.processOpenCommand(ctx, L"cd::test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, INVALID_HANDLE_VALUE);
}

TEST_F(ActionsTest, ProcessOpenCommand_InvalidCommand_OpensPanel) {
    NiceMock<MockFarApi> mockFar;
    NiceMock<MockFileSystem> mockFs;
    NiceMock<MockAliasStorage> mockStorage;

    std::vector<Alias> empty;
    EXPECT_CALL(mockStorage, getAll())
        .WillOnce(ReturnRef(empty));

    Effects effects(mockFar);
    Actions actions(mockStorage, mockFs, effects);

    auto result = actions.processOpenCommand(ctx, L"somecommand");
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(*result, INVALID_HANDLE_VALUE);
}