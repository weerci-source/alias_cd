#include <gtest/gtest.h>
#include "far2l_wrappers.h"
#include <vector>
#include <string>

// ============================================================================
// Глобальные переменные для моков ControlFunc
// ============================================================================
static HANDLE g_mock_control_h = nullptr;
static int g_mock_control_cmd = 0;
static int g_mock_control_p1 = 0;
static LONG_PTR g_mock_control_p2 = 0;

// ============================================================================
// Глобальные переменные для моков MessageFunc
// ============================================================================
static INT_PTR g_mock_msg_plugin = 0;
static DWORD g_mock_msg_flags = 0;
static const wchar_t* g_mock_msg_helptopic = nullptr;
static int g_mock_msg_items_count = 0;
static int g_mock_msg_buttons = 0;
static std::vector<std::wstring> g_mock_msg_items_copy;   // копия строк

// ============================================================================
// Моки для ControlFunc
// ============================================================================
int mockControlSuccess(HANDLE h, int cmd, int p1, LONG_PTR p2) {
    g_mock_control_h = h;
    g_mock_control_cmd = cmd;
    g_mock_control_p1 = p1;
    g_mock_control_p2 = p2;
    return 0;
}

int mockControlFailure(HANDLE h, int cmd, int p1, LONG_PTR p2) {
    return 1;
}

// ============================================================================
// Моки для MessageFunc
// ============================================================================
intptr_t mockMessageSuccess(INT_PTR PluginNumber, DWORD Flags, const wchar_t* HelpTopic,
                            const wchar_t* const* Items, int ItemsNumber, int ButtonsNumber) {
    g_mock_msg_plugin = PluginNumber;
    g_mock_msg_flags = Flags;
    g_mock_msg_helptopic = HelpTopic;
    g_mock_msg_items_count = ItemsNumber;
    g_mock_msg_buttons = ButtonsNumber;

    // Копируем строки, чтобы избежать висячих указателей
    g_mock_msg_items_copy.clear();
    for (int i = 0; i < ItemsNumber; ++i) {
        if (Items[i]) {
            g_mock_msg_items_copy.push_back(Items[i]);
        }
    }
    return 0;
}

intptr_t mockMessageFailure(INT_PTR PluginNumber, DWORD Flags, const wchar_t* HelpTopic,
                            const wchar_t* const* Items, int ItemsNumber, int ButtonsNumber) {
    return -1;
}

// ============================================================================
// Тесты для far2l::control
// ============================================================================

TEST(Far2lControlTest, CallsControlFuncWithCorrectArguments) {
    // Сбрасываем глобальные переменные
    g_mock_control_h = nullptr;
    g_mock_control_cmd = 0;
    g_mock_control_p1 = 0;
    g_mock_control_p2 = 0;

    HANDLE h = (HANDLE)0x1234;
    int cmd = 42, p1 = 100;
    void* p2 = (void*)0x5678;

    auto result = far2l::control(h, cmd, p1, p2, mockControlSuccess);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(g_mock_control_h, h);
    EXPECT_EQ(g_mock_control_cmd, cmd);
    EXPECT_EQ(g_mock_control_p1, p1);
    EXPECT_EQ(g_mock_control_p2, (LONG_PTR)p2);
}

TEST(Far2lControlTest, ReturnsErrorOnNullControlFunc) {
    auto result = far2l::control(nullptr, 0, 0, nullptr, nullptr);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::function_not_supported));
}

TEST(Far2lControlTest, ReturnsErrorOnInvalidHandle) {
    auto result = far2l::control(INVALID_HANDLE_VALUE, 0, 0, nullptr, mockControlSuccess);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::bad_file_descriptor));
}

TEST(Far2lControlTest, ReturnsErrorWhenControlFuncFails) {
    auto result = far2l::control((HANDLE)0x1, 0, 0, nullptr, mockControlFailure);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::operation_not_permitted));
}

// ============================================================================
// Тесты для far2l::message
// ============================================================================

TEST(Far2lMessageTest, CallsMessageFuncWithCorrectArguments) {
    // Сбрасываем глобальные переменные
    g_mock_msg_plugin = 0;
    g_mock_msg_flags = 0;
    g_mock_msg_helptopic = nullptr;
    g_mock_msg_items_copy.clear();
    g_mock_msg_items_count = 0;
    g_mock_msg_buttons = 0;

    INT_PTR pluginNumber = 42;
    DWORD flags = 0x1234;
    std::wstring title = L"Test Title";
    std::vector<std::wstring> items = {L"Item1", L"Item2", L"Item3"};
    int icon = 7;

    auto result = far2l::message(pluginNumber, flags, title, items, icon, mockMessageSuccess);
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(g_mock_msg_plugin, pluginNumber);
    EXPECT_EQ(g_mock_msg_flags, flags);
    // HelpTopic – указатель на title, но мы не проверяем его содержимое
    EXPECT_EQ(g_mock_msg_items_count, static_cast<int>(items.size()));
    EXPECT_EQ(g_mock_msg_buttons, icon);

    // Проверяем скопированные строки
    ASSERT_EQ(g_mock_msg_items_copy.size(), items.size());
    for (size_t i = 0; i < items.size(); ++i) {
        EXPECT_EQ(g_mock_msg_items_copy[i], items[i]);
    }
}

TEST(Far2lMessageTest, ReturnsErrorOnNullMessageFunc) {
    auto result = far2l::message(0, 0, L"", {}, 0, nullptr);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::function_not_supported));
}

TEST(Far2lMessageTest, ReturnsErrorWhenMessageFuncFails) {
    // Функция mockMessageFailure возвращает -1, что приведёт к ошибке
    auto result = far2l::message(0, 0, L"", {}, 0, mockMessageFailure);
    EXPECT_FALSE(result.has_value());
    // После исправления far2l::message (добавлена проверка возврата) ошибка должна быть io_error
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::io_error));
}

// Мок, который бросает исключение
int throwingControlFunc(HANDLE, int, int, LONG_PTR) {
    throw std::runtime_error("test exception");
}

intptr_t throwingMessageFunc(INT_PTR, DWORD, const wchar_t*, const wchar_t* const*, int, int) {
    throw std::runtime_error("test exception");
}

TEST(Far2lControlTest, HandlesExceptionFromControlFunc) {
    auto result = far2l::control((HANDLE)0x1, 0, 0, nullptr, throwingControlFunc);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::io_error));
}

TEST(Far2lMessageTest, HandlesExceptionFromMessageFunc) {
    auto result = far2l::message(0, 0, L"", {}, 0, throwingMessageFunc);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::io_error));
}