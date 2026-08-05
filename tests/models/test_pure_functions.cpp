#include <gtest/gtest.h>
#include "../../src/pure.h"

TEST(PureFunctionsTest, NormalizeCommand) {
    EXPECT_EQ(pure::normalizeCommand(L"Cd:Home"), L"cd:home");
    EXPECT_EQ(pure::normalizeCommand(L"  CD:  "), L"cd:");
    EXPECT_EQ(pure::normalizeCommand(L"cd:   test  "), L"cd:   test");
}

TEST(PureFunctionsTest, IsCdCommand) {
    EXPECT_TRUE(pure::isCdCommand(L"cd:home"));
    EXPECT_TRUE(pure::isCdCommand(L"cd:"));
    EXPECT_FALSE(pure::isCdCommand(L"home"));
}

TEST(PureFunctionsTest, ExtractArgument) {
    EXPECT_EQ(pure::extractArgument(L"cd:home"), L"home");
    EXPECT_EQ(pure::extractArgument(L"cd:  "), L"");
    EXPECT_EQ(pure::extractArgument(L"cd:"), L"");
}

TEST(PureFunctionsTest, ClassifyCommand) {
    using pure::CommandType;
    EXPECT_EQ(pure::classifyCommand(L""), CommandType::Panel);
    EXPECT_EQ(pure::classifyCommand(L":save"), CommandType::Save);
    EXPECT_EQ(pure::classifyCommand(L"home"), CommandType::Goto);
}