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