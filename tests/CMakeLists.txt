include(GoogleTest)

function(add_unit_test TEST_NAME)
    set(SOURCES ${ARGN})
    add_executable(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE utils_lib gtest_main gmock)
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
        ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    )
    target_compile_features(${TEST_NAME} PRIVATE cxx_std_23)
    gtest_discover_tests(${TEST_NAME})
endfunction()

add_library(utils_lib STATIC
    ../src/utils/utilites.cpp
    ../src/utils/writer.cpp
    ../src/models/alias.cpp
    ../src/models/alias_manager.cpp
    ../src/farwrapper/far2l_wrappers.cpp
    ../src/real/RealFarApi.cpp
    ../src/real/RealFileSystem.cpp
    ../src/real/RealWriter.cpp
    ../src/actions.cpp
    ../src/effects.cpp
    ../src/pure.cpp
)

# Базовые include-пути (общие для всех)
target_include_directories(utils_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/farwrapper
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/real
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
)

# Путь к far2l SDK – отдельный блок условий
if(FAR2L_SDK_PATH)
    target_include_directories(utils_lib PUBLIC ${FAR2L_SDK_PATH})
else()
    target_include_directories(utils_lib PUBLIC /home/dima/far2l-dev/src/far2l/far2sdk/)
endif()

target_compile_definitions(utils_lib PRIVATE
    WINPORT_DIRECT
    UNICODE
    FAR_DONT_USE_INTERNALS
)

target_compile_features(utils_lib PUBLIC cxx_std_23)

# Автоматическое обнаружение тестов
file(GLOB_RECURSE TEST_SOURCES "*.cpp")
set(TEST_TARGETS "")
foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_unit_test(${TEST_NAME} ${TEST_FILE})
    list(APPEND TEST_TARGETS ${TEST_NAME})
endforeach()

add_custom_target(tests DEPENDS ${TEST_TARGETS})