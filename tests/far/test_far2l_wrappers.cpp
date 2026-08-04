#include <gtest/gtest.h>
#include "far2l_wrappers.h"

// Заглушки
int mockControlSuccess(HANDLE h, int cmd, int p1, LONG_PTR p2) { return 0; }
int mockControlFailure(HANDLE h, int cmd, int p1, LONG_PTR p2) { return 1; }

TEST(Far2lControlTest, CallsControlFuncWithCorrectArguments) {
	HANDLE h = (HANDLE)0x1234;
	int cmd = 42, p1 = 100;
	void* p2 = (void*)0x5678;

	static HANDLE saved_h = nullptr;
	static int saved_cmd = 0, saved_p1 = 0;
	static LONG_PTR saved_p2 = 0;

	auto mock = [](HANDLE h, int cmd, int p1, LONG_PTR p2) -> int {
		saved_h = h; saved_cmd = cmd; saved_p1 = p1; saved_p2 = p2;
		return 0;
		};

	auto result = far2l::control(h, cmd, p1, p2, mock);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(saved_h, h);
	EXPECT_EQ(saved_cmd, cmd);
	EXPECT_EQ(saved_p1, p1);
	EXPECT_EQ(saved_p2, (LONG_PTR)p2);
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