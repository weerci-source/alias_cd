#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include "writer.h"    // src/utils/writer.h
#include "utilites.h"  // понадобится для readAllLines (если будем проверять)

namespace fs = std::filesystem;

class WriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаём временную папку для тестовых файлов
        testDir = fs::temp_directory_path() / "alias_cd_test";
        fs::create_directories(testDir);
        testFile = testDir / "test_output.txt";
    }

    void TearDown() override {
        // Удаляем временные файлы
        fs::remove_all(testDir);
    }

    fs::path testDir;
    fs::path testFile;
};

TEST_F(WriterTest, InitAndWriteString) {
    writer::Context ctx;
    auto initRes = writer::init(ctx, testFile.string());
    ASSERT_TRUE(initRes.has_value()) << "Init failed: " << initRes.error().message();

    auto writeRes = writer::write(ctx, "Hello, Writer!");
    ASSERT_TRUE(writeRes.has_value());
    writer::close(ctx);

    // Проверяем содержимое файла
    auto lines = writer::readAllLines(testFile.string());
    ASSERT_TRUE(lines.has_value());
    ASSERT_EQ(lines->size(), 1);
    EXPECT_EQ((*lines)[0], "Hello, Writer!"); // defaultFormatter добавляет '\n', но readAllLines её убирает
}

TEST_F(WriterTest, WriteVectorOfStrings) {
    writer::Context ctx;
    ASSERT_TRUE(writer::init(ctx, testFile.string()).has_value());

    std::vector<std::string> msgs = {"Line1", "Line2", "Line3"};
    auto writeRes = writer::write(ctx, msgs);
    ASSERT_TRUE(writeRes.has_value());
    writer::close(ctx);

    auto lines = writer::readAllLines(testFile.string());
    ASSERT_TRUE(lines.has_value());
    ASSERT_EQ(lines->size(), 3);
    EXPECT_EQ((*lines)[0], "Line1");
    EXPECT_EQ((*lines)[1], "Line2");
    EXPECT_EQ((*lines)[2], "Line3");
}

TEST_F(WriterTest, WriteWithCustomFormatter) {
    writer::Context ctx;
    ASSERT_TRUE(writer::init(ctx, testFile.string()).has_value());

    auto customFmt = [](std::string_view msg) {
        return "[" + std::string(msg) + "]";
    };
    ASSERT_TRUE(writer::write(ctx, "test", customFmt).has_value());
    writer::close(ctx);

    auto lines = writer::readAllLines(testFile.string());
    ASSERT_TRUE(lines.has_value());
    EXPECT_EQ((*lines)[0], "[test]");
}

TEST_F(WriterTest, OverwriteMode) {
    // Проверяем openOverwrite
    writer::Context ctx;
    ASSERT_TRUE(writer::openOverwrite(ctx, testFile.string()).has_value());
    ASSERT_TRUE(writer::write(ctx, "First content").has_value());
    writer::close(ctx);

    // Переоткрываем в режиме перезаписи
    ASSERT_TRUE(writer::openOverwrite(ctx, testFile.string()).has_value());
    ASSERT_TRUE(writer::write(ctx, "Second content").has_value());
    writer::close(ctx);

    auto lines = writer::readAllLines(testFile.string());
    ASSERT_TRUE(lines.has_value());
    ASSERT_EQ(lines->size(), 1);
    EXPECT_EQ((*lines)[0], "Second content");
}

TEST_F(WriterTest, WriteToClosedContext) {
    writer::Context ctx;
    // Не инициализируем файл
    auto writeRes = writer::write(ctx, "should fail");
    ASSERT_FALSE(writeRes.has_value());
    EXPECT_EQ(writeRes.error(), std::make_error_code(std::errc::bad_file_descriptor));
}

// ============================================================================
// Тесты для writer::makeContext
// ============================================================================

TEST_F(WriterTest, MakeContextReturnsValidContext) {
    writer::Context ctx = writer::makeContext();
    // Проверяем, что файл не открыт и formatter установлен по умолчанию
    EXPECT_FALSE(ctx.file.is_open());
    // Проверяем, что formatter не nullptr (можно вызвать)
    std::string formatted = ctx.frm("test");
    EXPECT_EQ(formatted, "test\n");
}

// ============================================================================
// Тесты для writer::init с ошибкой
// ============================================================================

TEST_F(WriterTest, InitFailsOnInvalidDirectory) {
    writer::Context ctx;
    std::string invalidPath = "/nonexistent/path/file.txt";
    auto result = writer::init(ctx, invalidPath);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

// ============================================================================
// Тесты для writer::write с ошибкой (контекст закрыт)
// ============================================================================

TEST_F(WriterTest, WriteFailsOnClosedContext) {
    writer::Context ctx;
    // Не открываем файл, сразу пишем
    auto result = writer::write(ctx, "test");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::bad_file_descriptor));
}

// ============================================================================
// Тесты для writer::readAllLines с ошибкой
// ============================================================================

TEST_F(WriterTest, ReadAllLinesFailsOnNonexistentFile) {
    auto result = writer::readAllLines("/nonexistent/file.txt");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

// ============================================================================
// Тесты для writer::openOverwrite с ошибкой
// ============================================================================

TEST_F(WriterTest, OpenOverwriteFailsOnInvalidPath) {
    writer::Context ctx;
    auto result = writer::openOverwrite(ctx, "/nonexistent/dir/file.txt");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST_F(WriterTest, WriteWithFormatterFailsOnClosedContext) {
    writer::Context ctx;
    auto customFmt = [](std::string_view msg) { return "[" + std::string(msg) + "]\n"; };
    auto result = writer::write(ctx, "test", customFmt);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::bad_file_descriptor));
}

TEST_F(WriterTest, WriteVectorFailsOnClosedContext) {
    writer::Context ctx;
    std::vector<std::string> msgs = {"a", "b"};
    auto result = writer::write(ctx, msgs);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::bad_file_descriptor));
}

TEST_F(WriterTest, ReadAllLinesWithEmptyLinesSkipsThem) {
    std::string path = testFile.string();
    std::ofstream file(path);
    file << "line1\n\nline2\n";
    file.close();
    auto result = writer::readAllLines(path);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 2);
    EXPECT_EQ((*result)[0], "line1");
    EXPECT_EQ((*result)[1], "line2");
}

TEST_F(WriterTest, ReadAllLinesVersion2WithEmptyLines) {
    std::string path = testFile.string();
    std::ofstream file(path);
    file << "line1\n\nline2\n";
    file.close();
    std::vector<std::string> out;
    auto result = writer::readAllLines(path, out);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(out.size(), 2);
    EXPECT_EQ(out[0], "line1");
    EXPECT_EQ(out[1], "line2");
}
