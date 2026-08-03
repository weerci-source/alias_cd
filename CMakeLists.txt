cmake_minimum_required(VERSION 3.31)
project(alias_cd)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/farwrapper/far2l_wrappers.cpp
    src/models/alias.cpp
    src/models/alias_manager.cpp
    src/utils/writer.cpp
    src/utils/utilites.cpp
    alias_cd.cpp
)
add_library(alias_cd MODULE ${SOURCES})

target_compile_definitions(alias_cd
	PRIVATE
		-DWINPORT_DIRECT
		-DUNICODE
		-DFAR_DONT_USE_INTERNALS
)

target_include_directories(alias_cd
	PRIVATE /home/dima/far2l-dev/src/far2l/far2sdk/
)

get_target_property(INC_DIRS alias_cd INCLUDE_DIRECTORIES)

set_target_properties(alias_cd
	PROPERTIES
		PREFIX ""
		SUFFIX ".far-plug-wide"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET alias_cd POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:alias_cd>
    )
endif()