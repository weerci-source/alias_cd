#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "../../src/models/alias.h"
#include "../../src/models/alias_manager.h"
#include "../../src/interfaces/IFileSystem.h"
#include "../../src/interfaces/IWriter.h"
#include "../../src/utils/utilites.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::SetArgReferee;

namespace fs = std::filesystem;

// ============================================================================
// Моки для IFileSystem и IWriter
// ============================================================================

class MockFileSystem : public IFileSystem {
public:
    MOCK_METHOD(std::expected<std::wstring, std::error_code>, getCurrentDir, (), (noexcept, override));
    MOCK_METHOD(std::expected<void, std::error_code>, setCurrentDir, (const std::wstring&), (noexcept, override));
};

class MockWriter : public IWriter {
public:
    MOCK_METHOD(std::expected<void, std::error_code>, init, (const std::string&), (noexcept, override));
    MOCK_METHOD(std::expected<void, std::error_code>, write, (const std::string&), (noexcept, override));
    MOCK_METHOD(std::expected<void, std::error_code>, write, (const std::vector<std::string>&), (noexcept, override));
    MOCK_METHOD(void, close, (), (noexcept, override));
    MOCK_METHOD(std::expected<std::vector<std::string>, std::error_code>, readAllLines, (const std::string&), (noexcept, override));
    MOCK_METHOD(std::expected<void, std::error_code>, openOverwrite, (const std::string&), (noexcept, override));
};

// ============================================================================
// Фикстура для тестов AliasManager
// ============================================================================

class AliasManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаём временную папку для тестов (используется только для проверки, но моки всё равно подменяют)
        testDir = fs::temp_directory_path() / "alias_manager_test";
        fs::create_directories(testDir);
        testFile = testDir / "aliases";

        // Создаём моки
        mockFs = std::make_unique<NiceMock<MockFileSystem>>();
        mockWriter = std::make_unique<NiceMock<MockWriter>>();

        // Создаём AliasManager с моками
        manager = std::make_unique<AliasManager>(*mockFs, *mockWriter);
    }

    void TearDown() override {
        fs::remove_all(testDir);
    }

    std::unique_ptr<MockFileSystem> mockFs;
    std::unique_ptr<MockWriter> mockWriter;
    std::unique_ptr<AliasManager> manager;
    fs::path testDir;
    fs::path testFile;
};

// ============================================================================
// Тесты для AliasManager
// ============================================================================

TEST_F(AliasManagerTest, LoadEmptyFileWhenFileDoesNotExist) {
    // Мок readAllLines возвращает ошибку "no such file"
    EXPECT_CALL(*mockWriter, readAllLines(_))
        .WillOnce(Return(std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory))));

    auto result = manager->load();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(manager->getAll().empty());
}

TEST_F(AliasManagerTest, LoadValidLines) {
    std::vector<std::string> lines = {"home=/home/user", "work=/work/project"};
    EXPECT_CALL(*mockWriter, readAllLines(_))
        .WillOnce(Return(std::expected<std::vector<std::string>, std::error_code>(lines)));

    auto result = manager->load();
    ASSERT_TRUE(result.has_value());

    const auto& aliases = manager->getAll();
    ASSERT_EQ(aliases.size(), 2);
    EXPECT_EQ(aliases[0].name, L"home");
    EXPECT_EQ(aliases[0].path, L"/home/user");
    EXPECT_EQ(aliases[1].name, L"work");
    EXPECT_EQ(aliases[1].path, L"/work/project");
}

TEST_F(AliasManagerTest, LoadIgnoresInvalidLines) {
    std::vector<std::string> lines = {
        "home=/home/user",
        "# comment",
        "invalid_line",
        "work=/work/project",
        "=",
        "name=",
        "=/path"
    };
    EXPECT_CALL(*mockWriter, readAllLines(_))
        .WillOnce(Return(std::expected<std::vector<std::string>, std::error_code>(lines)));

    auto result = manager->load();
    ASSERT_TRUE(result.has_value());

    const auto& aliases = manager->getAll();
    ASSERT_EQ(aliases.size(), 2);
    EXPECT_EQ(aliases[0].name, L"home");
    EXPECT_EQ(aliases[1].name, L"work");
}

TEST_F(AliasManagerTest, SaveWritesAliasesToFile) {
    // Добавляем алиасы вручную (через addOrUpdate)
    Alias a1{L"home", L"/home/user"};
    Alias a2{L"work", L"/work/project"};
    manager->addOrUpdate(a1);
    manager->addOrUpdate(a2);

    // Ожидаем, что openOverwrite будет вызван, и write будет вызван дважды
    EXPECT_CALL(*mockWriter, openOverwrite(_))
        .WillOnce(Return(std::expected<void, std::error_code>{}));
    EXPECT_CALL(*mockWriter, write("home=/home/user"))
        .WillOnce(Return(std::expected<void, std::error_code>{}));
    EXPECT_CALL(*mockWriter, write("work=/work/project"))
        .WillOnce(Return(std::expected<void, std::error_code>{}));

    auto saveResult = manager->save();
    ASSERT_TRUE(saveResult.has_value());
}

TEST_F(AliasManagerTest, AddOrUpdateAddsNewAlias) {
    Alias a{L"test", L"/test/path"};
    auto result = manager->addOrUpdate(a);
    ASSERT_TRUE(result.has_value());

    const auto& aliases = manager->getAll();
    ASSERT_EQ(aliases.size(), 1);
    EXPECT_EQ(aliases[0].name, L"test");
    EXPECT_EQ(aliases[0].path, L"/test/path");
}

TEST_F(AliasManagerTest, AddOrUpdateUpdatesExistingAlias) {
    Alias a1{L"test", L"/old/path"};
    manager->addOrUpdate(a1);
    Alias a2{L"test", L"/new/path"};
    auto result = manager->addOrUpdate(a2);
    ASSERT_TRUE(result.has_value());

    const auto& aliases = manager->getAll();
    ASSERT_EQ(aliases.size(), 1);
    EXPECT_EQ(aliases[0].name, L"test");
    EXPECT_EQ(aliases[0].path, L"/new/path");
}

TEST_F(AliasManagerTest, AddOrUpdateWithInvalidAliasReturnsError) {
    Alias invalid{L"", L"/path"};
    auto result = manager->addOrUpdate(invalid);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::invalid_argument));
}

TEST_F(AliasManagerTest, RemoveExistingAlias) {
    Alias a{L"test", L"/test/path"};
    manager->addOrUpdate(a);
    auto result = manager->remove(L"test");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(manager->getAll().empty());
}

TEST_F(AliasManagerTest, RemoveNonExistingAliasReturnsError) {
    auto result = manager->remove(L"nonexistent");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(AliasManagerTest, FindExistingAlias) {
    Alias a{L"test", L"/test/path"};
    manager->addOrUpdate(a);
    auto result = manager->find(L"test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)->name, L"test");
    EXPECT_EQ((*result)->path, L"/test/path");
}

TEST_F(AliasManagerTest, FindNonExistingAliasReturnsError) {
    auto result = manager->find(L"nonexistent");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(AliasManagerTest, ClearRemovesAllAliasesAndSaves) {
    Alias a1{L"home", L"/home/user"};
    Alias a2{L"work", L"/work/project"};
    manager->addOrUpdate(a1);
    manager->addOrUpdate(a2);

    // Ожидаем, что save будет вызван (clear вызывает save)
    EXPECT_CALL(*mockWriter, openOverwrite(_))
        .WillOnce(Return(std::expected<void, std::error_code>{}));
    // Запись пустого списка — write не будет вызван, так как aliases_ пуст
    // Но save может вызвать write для каждого алиаса, но их нет

    auto result = manager->clear();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(manager->getAll().empty());
}

TEST_F(AliasManagerTest, InitWithNonExistingDirectoryReturnsError) {
    // Не нужно, так как мы используем моки, но можно проверить, что init вызывает openOverwrite
    EXPECT_CALL(*mockWriter, openOverwrite(_))
        .WillOnce(Return(std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory))));

    auto result = manager->init(L"/nonexistent/path/aliases");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AliasManagerTest, LoadEmptyFileGivesEmptyList) {
    std::vector<std::string> emptyLines;
    EXPECT_CALL(*mockWriter, readAllLines(_))
        .WillOnce(Return(std::expected<std::vector<std::string>, std::error_code>(emptyLines)));

    auto result = manager->load();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(manager->getAll().empty());
}

TEST_F(AliasManagerTest, LoadIgnoresInvalidUtf8Lines) {
    std::vector<std::string> lines = {"\xFF\xFF", "valid=path"};
    EXPECT_CALL(*mockWriter, readAllLines(_))
        .WillOnce(Return(std::expected<std::vector<std::string>, std::error_code>(lines)));

    auto result = manager->load();
    ASSERT_TRUE(result.has_value());

    const auto& aliases = manager->getAll();
    ASSERT_EQ(aliases.size(), 1);
    EXPECT_EQ(aliases[0].name, L"valid");
    EXPECT_EQ(aliases[0].path, L"path");
}