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