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

TEST(PureFunctionsTest, SetContextAndIsInitialized) {
    EXPECT_FALSE(pure::isInitialized());
    
    PluginContext ctx;
    ctx.Info.ModuleNumber = 42;
    ctx.Info.Control = nullptr;
    ctx.Info.Message = nullptr;
    ctx.FSF = nullptr;
    
    pure::setContext(std::move(ctx));
    EXPECT_TRUE(pure::isInitialized());
    const PluginContext& retrieved = pure::context();
    EXPECT_EQ(retrieved.Info.ModuleNumber, 42);
}

TEST(PureFunctionsTest, ClassifyCommand_Unknown) {
    using pure::CommandType;
    // Проверяем, что для строки, не начинающейся с ':' и не пустой, возвращается Goto
    EXPECT_EQ(pure::classifyCommand(L"some"), CommandType::Goto);
    // Для пустой – Panel
    EXPECT_EQ(pure::classifyCommand(L""), CommandType::Panel);
    // Для начинающейся с ':' – Save
    EXPECT_EQ(pure::classifyCommand(L":save"), CommandType::Save);
    // Для чего-то ещё, например, с пробелами? Но это уже покрыто.
}
