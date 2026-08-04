# Раннее включение модуля GoogleTest (ОБЯЗАТЕЛЬНО ДО всего остального)
include(GoogleTest)

# Вспомогательная функция для добавления тестового исполняемого файла
function(add_unit_test TEST_NAME)
    set(SOURCES ${ARGN})
    add_executable(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE utils_lib gtest_main)
    target_compile_features(${TEST_NAME} PRIVATE cxx_std_23)
    gtest_discover_tests(${TEST_NAME})
endfunction()

# Библиотека с тестируемыми модулями (теперь включает alias и alias_manager)
add_library(utils_lib STATIC
    ../src/utils/utilites.cpp
    ../src/utils/writer.cpp
    ../src/models/alias.cpp
    ../src/models/alias_manager.cpp
)
target_include_directories(utils_lib PUBLIC ../src/utils)
target_compile_features(utils_lib PUBLIC cxx_std_23)

# Регистрация тестов
add_unit_test(test_utf_conversions utils/test_utf_conversions.cpp)
add_unit_test(test_writer utils/test_writer.cpp)
add_unit_test(test_alias_manager models/test_alias_manager.cpp)
add_unit_test(test_utilites utils/test_utilites.cpp)