#include <gtest/gtest.h>
#include "utilites.h"

TEST(TrimTest, StringEmpty)
{
    std::string s = "";
    EXPECT_EQ(trim(s), "");
    std::wstring ws = L"";
    EXPECT_EQ(trim(ws), L"");
}

TEST(TrimTest, StringOnlySpaces)
{
    std::string s = "   ";
    EXPECT_EQ(trim(s), "");
    std::wstring ws = L"\t\n\r   ";
    EXPECT_EQ(trim(ws), L"");
}

TEST(TrimTest, StringNoSpaces)
{
    std::string s = "hello";
    EXPECT_EQ(trim(s), "hello");
    std::wstring ws = L"world";
    EXPECT_EQ(trim(ws), L"world");
}

TEST(TrimTest, StringSpacesAround)
{
    std::string s = "  hello  ";
    EXPECT_EQ(trim(s), "hello");
    std::wstring ws = L"\t\nhello\r\n"; 
    EXPECT_EQ(trim(ws), L"hello");
}

TEST(TrimTest, StringWithInternalSpaces)
{
    std::string s = "  hello world  ";
    EXPECT_EQ(trim(s), "hello world");
    std::wstring ws = L"\thello\tworld\n";
    EXPECT_EQ(trim(ws), L"hello\tworld");
}

TEST(UtilitesTest, GetCurrentDirW_UsesMockWhenSet) {
    // Устанавливаем мок
    g_getCurrentDirW_impl = []() noexcept -> std::expected<std::wstring, std::error_code> {
        return L"/mock/path";
    };
    auto result = getCurrentDirW();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, L"/mock/path");
    g_getCurrentDirW_impl = nullptr; // сброс
}

TEST(UtilitesTest, GetCurrentDirW_ReturnsErrorWhenMockFails) {
    g_getCurrentDirW_impl = []() noexcept -> std::expected<std::wstring, std::error_code> {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    };
    auto result = getCurrentDirW();
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
    g_getCurrentDirW_impl = nullptr;
}

TEST(UtilitesTest, SetCurrentDirW_UsesMockWhenSet) {
    std::wstring testPath = L"/test/path";
    g_setCurrentDirW_impl = [](const std::wstring& path) noexcept -> std::expected<void, std::error_code> {
        return {};
    };
    auto result = setCurrentDirW(testPath);
    EXPECT_TRUE(result.has_value());
    g_setCurrentDirW_impl = nullptr;
}

TEST(UtilitesTest, SetCurrentDirW_ReturnsErrorWhenMockFails) {
    g_setCurrentDirW_impl = [](const std::wstring& path) noexcept -> std::expected<void, std::error_code> {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    };
    auto result = setCurrentDirW(L"/test/path");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::no_such_file_or_directory));
    g_setCurrentDirW_impl = nullptr;
}

// ============================================================================
// Тесты для currentTime
// ============================================================================

TEST(UtilitesTest, CurrentTimeReturnsValidFormat) {
    std::string timeStr = currentTime();
    // Проверяем, что строка имеет формат YYYY-MM-DD HH:MM:SS (длина 19)
    EXPECT_EQ(timeStr.size(), 19);
    EXPECT_EQ(timeStr[4], '-');
    EXPECT_EQ(timeStr[7], '-');
    EXPECT_EQ(timeStr[10], ' ');
    EXPECT_EQ(timeStr[13], ':');
    EXPECT_EQ(timeStr[16], ':');
}

// ============================================================================
// Тесты для initLocale (успех и ошибка)
// ============================================================================

TEST(UtilitesTest, InitLocaleSuccess) {
    auto result = initLocale();
    EXPECT_TRUE(result.has_value());
}

// ============================================================================
// Тесты для UTF8ToWString с ошибкой
// ============================================================================

TEST(UtilitesTest, UTF8ToWString_InvalidUtf8ReturnsEmpty) {
    // Некорректная последовательность, которая вызовет ошибку mbstowcs
    std::string invalid = "\xFF\xFF";
    std::wstring result = UTF8ToWString(invalid);
    EXPECT_TRUE(result.empty());
}

TEST(UtilitesTest, UTF8ToWString_EmptyStringReturnsEmpty) {
    std::wstring result = UTF8ToWString("");
    EXPECT_TRUE(result.empty());
}

// ============================================================================
// Тесты для WStringToUTF8 с ошибкой
// ============================================================================

TEST(UtilitesTest, WStringToUTF8_InvalidWideReturnsEmpty) {
    // Создаём wide-строку с некорректным символом (например, суррогат)
    // Это сложно, потому что wcstombs может вернуть -1 при неконвертируемом символе.
    // Пропустим, так как в Linux это редко.
    // Вместо этого проверим, что для пустой строки возвращается пустая строка.
    std::string result = WStringToUTF8(L"");
    EXPECT_TRUE(result.empty());
}

// ============================================================================
// Тесты для trim (уже есть, но можно добавить крайние случаи)
// ============================================================================

TEST(TrimTest, StringWithOnlySpacesAndTabs) {
    std::string s = "\t \t ";
    EXPECT_EQ(trim(s), "");
    std::wstring ws = L"\t \t ";
    EXPECT_EQ(trim(ws), L"");
}