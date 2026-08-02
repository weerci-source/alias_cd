# tests/CMakeLists.txt

# Исполняемый файл для тестов логгера
add_executable(test_logger
    test_logger.cpp
)

# Линкуем с библиотекой логгера и Google Test
target_link_libraries(test_logger
    logger_lib
    gtest_main
)

# Добавляем тест в CTest
add_test(NAME test_logger COMMAND test_logger)

# Опционально: если хотите отдельные тесты для utils, добавьте аналогично
# add_executable(test_utils test_utils.cpp)
# target_link_libraries(test_utils logger_lib gtest_main)
# add_test(NAME test_utils COMMAND test_utils)

# Создаём цель для запуска всех тестов
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_logger
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests"
)