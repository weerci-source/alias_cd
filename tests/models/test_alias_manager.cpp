#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/models/alias.h"
#include "../../src/models/alias_manager.h"
#include "../../src/utils/utilites.h"   // для initLocale

namespace fs = std::filesystem;

// ============================================================================
// Фикстура для тестов функций alias (устанавливает локаль)
// ============================================================================
class AliasFunctionsTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        auto result = initLocale();
        ASSERT_TRUE(result.has_value()) << "Failed to set locale";
    }
};

// ============================================================================
// Тесты для чистых функций (alias.cpp)
// ============================================================================

TEST_F(AliasFunctionsTest, IsValidAlias) {
    Alias a1{L"home", L"/home/user"};
    EXPECT_TRUE(isValidAlias(a1));

    Alias a2{L"", L"/path"};
    EXPECT_FALSE(isValidAlias(a2));

    Alias a3{L"name", L""};
    EXPECT_FALSE(isValidAlias(a3));

    Alias a4{L"", L""};
    EXPECT_FALSE(isValidAlias(a4));
}

TEST_F(AliasFunctionsTest, ParseValidLine) {
    std::string line = "home=/home/user";
    auto result = parseAliasLine(line);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, L"home");
    EXPECT_EQ(result->path, L"/home/user");

    line = "  work  =  /work/project  ";
    result = parseAliasLine(line);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, L"work");
    EXPECT_EQ(result->path, L"/work/project");

    line = "привет=/путь/к/папке";
    result = parseAliasLine(line);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, L"привет");
    EXPECT_EQ(result->path, L"/путь/к/папке");
}

TEST_F(AliasFunctionsTest, ParseInvalidLines) {
    auto result = parseAliasLine("");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::invalid_argument));

    result = parseAliasLine("# comment");
    EXPECT_FALSE(result.has_value());

    result = parseAliasLine("invalid");
    EXPECT_FALSE(result.has_value());

    result = parseAliasLine("=/path");
    EXPECT_FALSE(result.has_value());

    result = parseAliasLine("name=");
    EXPECT_FALSE(result.has_value());

    result = parseAliasLine("=");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AliasFunctionsTest, SerializeAlias) {
    Alias a{L"home", L"/home/user"};
    std::string serialized = serializeAlias(a);
    EXPECT_EQ(serialized, "home=/home/user");

    Alias b{L"work", L"/work/project"};
    EXPECT_EQ(serializeAlias(b), "work=/work/project");

    Alias c{L"привет", L"/путь/к/папке"};
    EXPECT_EQ(serializeAlias(c), "привет=/путь/к/папке");
}

// ============================================================================
// Тесты для AliasManager (используем временные файлы)
// ============================================================================

class AliasManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "alias_manager_test";
        fs::create_directories(testDir);
        testFile = testDir / "aliases";

        auto& mgr = AliasManager::Instance();
        auto initResult = mgr.init(testFile.wstring());
        ASSERT_TRUE(initResult.has_value()) << "Init failed: " << initResult.error().message();
        mgr.clear();
    }

    void TearDown() override {
        fs::remove_all(testDir);
    }

    void writeLines(const std::vector<std::string>& lines) {
        std::ofstream file(testFile);
        for (const auto& line : lines) {
            file << line << '\n';
        }
        file.close();
    }

    std::vector<std::string> readLines() {
        std::vector<std::string> lines;
        std::ifstream file(testFile);
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    fs::path testDir;
    fs::path testFile;
};

TEST_F(AliasManagerTest, LoadEmptyFileWhenFileDoesNotExist) {
    auto& mgr = AliasManager::Instance();
    auto result = mgr.load();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(mgr.getAll().empty());
}

TEST_F(AliasManagerTest, LoadValidLines) {
    writeLines({"home=/home/user", "work=/work/project"});

    auto& mgr = AliasManager::Instance();
    auto result = mgr.load();
    ASSERT_TRUE(result.has_value());

    const auto& aliases = mgr.getAll();
    ASSERT_EQ(aliases.size(), 2);
    EXPECT_EQ(aliases[0].name, L"home");
    EXPECT_EQ(aliases[0].path, L"/home/user");
    EXPECT_EQ(aliases[1].name, L"work");
    EXPECT_EQ(aliases[1].path, L"/work/project");
}

TEST_F(AliasManagerTest, LoadIgnoresInvalidLines) {
    writeLines({
        "home=/home/user",
        "# comment",
        "invalid_line",
        "work=/work/project",
        "=",
        "name=",
        "=/path"
    });

    auto& mgr = AliasManager::Instance();
    auto result = mgr.load();
    ASSERT_TRUE(result.has_value());

    const auto& aliases = mgr.getAll();
    ASSERT_EQ(aliases.size(), 2);
    EXPECT_EQ(aliases[0].name, L"home");
    EXPECT_EQ(aliases[1].name, L"work");
}

TEST_F(AliasManagerTest, SaveWritesAliasesToFile) {
    auto& mgr = AliasManager::Instance();
    Alias a1{L"home", L"/home/user"};
    Alias a2{L"work", L"/work/project"};
    mgr.addOrUpdate(a1);
    mgr.addOrUpdate(a2);

    auto saveResult = mgr.save();
    ASSERT_TRUE(saveResult.has_value());

    auto lines = readLines();
    ASSERT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0], "home=/home/user");
    EXPECT_EQ(lines[1], "work=/work/project");
}

TEST_F(AliasManagerTest, AddOrUpdateAddsNewAlias) {
    auto& mgr = AliasManager::Instance();
    Alias a{L"test", L"/test/path"};
    auto result = mgr.addOrUpdate(a);
    ASSERT_TRUE(result.has_value());

    const auto& aliases = mgr.getAll();
    ASSERT_EQ(aliases.size(), 1);
    EXPECT_EQ(aliases[0].name, L"test");
    EXPECT_EQ(aliases[0].path, L"/test/path");
}

TEST_F(AliasManagerTest, AddOrUpdateUpdatesExistingAlias) {
    auto& mgr = AliasManager::Instance();
    Alias a1{L"test", L"/old/path"};
    mgr.addOrUpdate(a1);
    Alias a2{L"test", L"/new/path"};
    auto result = mgr.addOrUpdate(a2);
    ASSERT_TRUE(result.has_value());

    const auto& aliases = mgr.getAll();
    ASSERT_EQ(aliases.size(), 1);
    EXPECT_EQ(aliases[0].name, L"test");
    EXPECT_EQ(aliases[0].path, L"/new/path");
}

TEST_F(AliasManagerTest, AddOrUpdateWithInvalidAliasReturnsError) {
    auto& mgr = AliasManager::Instance();
    Alias invalid{L"", L"/path"};
    auto result = mgr.addOrUpdate(invalid);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::invalid_argument));
}

TEST_F(AliasManagerTest, RemoveExistingAlias) {
    auto& mgr = AliasManager::Instance();
    Alias a{L"test", L"/test/path"};
    mgr.addOrUpdate(a);
    auto result = mgr.remove(L"test");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(mgr.getAll().empty());
}

TEST_F(AliasManagerTest, RemoveNonExistingAliasReturnsError) {
    auto& mgr = AliasManager::Instance();
    auto result = mgr.remove(L"nonexistent");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(AliasManagerTest, FindExistingAlias) {
    auto& mgr = AliasManager::Instance();
    Alias a{L"test", L"/test/path"};
    mgr.addOrUpdate(a);
    auto result = mgr.find(L"test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)->name, L"test");
    EXPECT_EQ((*result)->path, L"/test/path");
}

TEST_F(AliasManagerTest, FindNonExistingAliasReturnsError) {
    auto& mgr = AliasManager::Instance();
    auto result = mgr.find(L"nonexistent");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(AliasManagerTest, ClearRemovesAllAliasesAndSaves) {
    auto& mgr = AliasManager::Instance();
    Alias a1{L"home", L"/home/user"};
    Alias a2{L"work", L"/work/project"};
    mgr.addOrUpdate(a1);
    mgr.addOrUpdate(a2);
    auto result = mgr.clear();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(mgr.getAll().empty());

    auto lines = readLines();
    EXPECT_TRUE(lines.empty());
}

TEST_F(AliasManagerTest, InitWithNonExistingDirectoryReturnsError) {
    auto& mgr = AliasManager::Instance();
    std::wstring badPath = L"/nonexistent/path/aliases";
    auto result = mgr.init(badPath);
    EXPECT_FALSE(result.has_value());
}

TEST_F(AliasManagerTest, LoadEmptyFileGivesEmptyList) {
    std::ofstream file(testFile); 
    file.close();

    auto& mgr = AliasManager::Instance();
    auto result = mgr.load();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(mgr.getAll().empty());
}