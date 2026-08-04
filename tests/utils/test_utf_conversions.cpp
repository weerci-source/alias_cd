#include <gtest/gtest.h>
#include "utilites.h"   // путь: src/utils/utilites.h
#include <clocale>

// Фикстура для установки локали, т.к. функции используют mbstowcs/wcstombs
class UtfConversionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Устанавливаем UTF-8 локаль, чтобы конверсии работали корректно
        std::setlocale(LC_ALL, "en_US.UTF-8");
    }
};

TEST_F(UtfConversionTest, EmptyString) {
    EXPECT_TRUE(UTF8ToWString("").empty());
    EXPECT_TRUE(WStringToUTF8(L"").empty());
}

TEST_F(UtfConversionTest, AsciiRoundtrip) {
    std::string ascii = "Hello, World!";
    std::wstring wide = UTF8ToWString(ascii);
    EXPECT_EQ(wide, L"Hello, World!");
    EXPECT_EQ(WStringToUTF8(wide), ascii);
}

TEST_F(UtfConversionTest, RussianRoundtrip) {
    std::string utf8 = "Привет, мир!";
    std::wstring wide = UTF8ToWString(utf8);
    // Ожидаем, что широкая строка будет содержать те же символы
    EXPECT_EQ(wide.size(), 12); // 12 символов (включая пробел и запятую)
    std::string back = WStringToUTF8(wide);
    EXPECT_EQ(back, utf8);
}

TEST_F(UtfConversionTest, SpecialSymbols) {
    std::string utf8 = "© € → λ";
    std::wstring wide = UTF8ToWString(utf8);
    EXPECT_EQ(wide.size(), 7); // 4 символа + 3 пробела
    EXPECT_EQ(WStringToUTF8(wide), utf8);
}

TEST_F(UtfConversionTest, InvalidUtf8ReturnsEmpty) {
    // Некорректная последовательность (0xFF) приводит к ошибке преобразования
    std::wstring res = UTF8ToWString("\xFF\xFF");
    // Ожидается пустая строка, так как mbstowcs вернёт -1
    EXPECT_TRUE(res.empty());
}