#include <gtest/gtest.h>
#include "utilites.h"

TEST(TrimTest, StringEmpty) {
    std::string s = "";
    EXPECT_EQ(trim(s), "");
    std::wstring ws = L"";
    EXPECT_EQ(trim(ws), L"");
}

TEST(TrimTest, StringOnlySpaces) {
    std::string s = "   ";
    EXPECT_EQ(trim(s), "");
    std::wstring ws = L"\t\n\r   ";
    EXPECT_EQ(trim(ws), L"");
}

TEST(TrimTest, StringNoSpaces) {
    std::string s = "hello";
    EXPECT_EQ(trim(s), "hello");
    std::wstring ws = L"world";
    EXPECT_EQ(trim(ws), L"world");
}

TEST(TrimTest, StringSpacesAround) {
    std::string s = "  hello  ";
    EXPECT_EQ(trim(s), "hello");
    std::wstring ws = L"\t\nhello\r\n"; 
    EXPECT_EQ(trim(ws), L"hello");
}

TEST(TrimTest, StringWithInternalSpaces) {
    std::string s = "  hello world  ";
    EXPECT_EQ(trim(s), "hello world");
    std::wstring ws = L"\thello\tworld\n";
    EXPECT_EQ(trim(ws), L"hello\tworld");
}

TEST(TrimTest, StringWithOnlySpacesAndTabs) {
    std::string s = "\t \t ";
    EXPECT_EQ(trim(s), "");
    std::wstring ws = L"\t \t ";
    EXPECT_EQ(trim(ws), L"");
}

TEST(UtilitesTest, CurrentTimeReturnsValidFormat) {
    std::string timeStr = currentTime();
    EXPECT_EQ(timeStr.size(), 19);
    EXPECT_EQ(timeStr[4], '-');
    EXPECT_EQ(timeStr[7], '-');
    EXPECT_EQ(timeStr[10], ' ');
    EXPECT_EQ(timeStr[13], ':');
    EXPECT_EQ(timeStr[16], ':');
}

TEST(UtilitesTest, InitLocaleSuccess) {
    auto result = initLocale();
    EXPECT_TRUE(result.has_value());
}

TEST(UtilitesTest, UTF8ToWString_InvalidUtf8ReturnsEmpty) {
    std::string invalid = "\xFF\xFF";
    std::wstring result = UTF8ToWString(invalid);
    EXPECT_TRUE(result.empty());
}

TEST(UtilitesTest, UTF8ToWString_EmptyStringReturnsEmpty) {
    std::wstring result = UTF8ToWString("");
    EXPECT_TRUE(result.empty());
}

TEST(UtilitesTest, WStringToUTF8_EmptyStringReturnsEmpty) {
    std::string result = WStringToUTF8(L"");
    EXPECT_TRUE(result.empty());
}