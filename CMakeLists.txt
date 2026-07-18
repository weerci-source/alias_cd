project(alias_cd)

set(SOURCES alias_cd.cpp)

add_library(alias_cd MODULE ${SOURCES})

target_link_libraries(alias_cd far2l)

target_compile_definitions(alias_cd PRIVATE -DWINPORT_DIRECT -DUNICODE -DFAR_DONT_USE_INTERNALS)

target_include_directories(alias_cd PRIVATE
    ${CMAKE_SOURCE_DIR}/far2l/far2sdk
    ${CMAKE_SOURCE_DIR}/far2l/WinPort
)

set_target_properties(alias_cd PROPERTIES
    PREFIX ""
    SUFFIX ".far-plug-wide")