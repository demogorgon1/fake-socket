cmake_minimum_required (VERSION 3.19)

project(fake-socket)

macro(fake_socket_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})
    if(DEFINED ENV{${OPTION_NAME}})
        # Allow overriding the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()
    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()
    message(STATUS "${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

fake_socket_option(FAKE_SOCKET_TEST "Build tests." ON)

enable_testing()

include(FetchContent)
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  ${CMAKE_MODULE_PATH})
  
# -------------------

if(MSVC)
	add_compile_options(/bigobj /MP)
endif()

add_subdirectory(lib)
add_subdirectory(include)

if(FAKE_SOCKET_TEST)
	add_subdirectory(test)
endif()