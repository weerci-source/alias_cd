# Подключаем GoogleTest
include(GoogleTest)

# Функция для создания тестового исполняемого файла
function(add_unit_test TEST_NAME)
    set(SOURCES ${ARGN})
    add_executable(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE utils_lib gtest_main)
    target_compile_features(${TEST_NAME} PRIVATE cxx_std_23)
    gtest_discover_tests(${TEST_NAME})
endfunction()

# Общая библиотека с тестируемыми модулями (добавляйте сюда новые файлы по мере необходимости)
add_library(utils_lib STATIC
    ../src/utils/utilites.cpp
    ../src/utils/writer.cpp
    ../src/models/alias.cpp
    ../src/models/alias_manager.cpp
    ../src/farwrapper/far2l_wrappers.cpp
    ../alias_cd.cpp
)

target_include_directories(utils_lib PUBLIC 
    ../src/utils
    ../src/farwrapper
    /home/dima/far2l-dev/src/far2l/far2sdk/
)
target_compile_definitions(utils_lib PRIVATE
    WINPORT_DIRECT
    UNICODE
    FAR_DONT_USE_INTERNALS
)
target_compile_features(utils_lib PUBLIC cxx_std_23)

# ---- АВТОМАТИЧЕСКОЕ ОБНАРУЖЕНИЕ ТЕСТОВ ----
# Находим все .cpp файлы в подпапках tests/ (рекурсивно)
file(GLOB_RECURSE TEST_SOURCES 
    "*.cpp"
)

# Исключаем возможные служебные файлы (если понадобится)
# list(REMOVE_ITEM TEST_SOURCES "some_excluded_file.cpp")

set(TEST_TARGETS "")

foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)  # имя без расширения
    add_unit_test(${TEST_NAME} ${TEST_FILE})
    list(APPEND TEST_TARGETS ${TEST_NAME})
endforeach()

# Цель для сборки всех тестов (используется в tasks.json)
add_custom_target(tests DEPENDS ${TEST_TARGETS})