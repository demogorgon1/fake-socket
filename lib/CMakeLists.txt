cmake_minimum_required (VERSION 3.19)

file(GLOB_RECURSE C_FILES "*.c")
file(GLOB_RECURSE H_FILES "*.h")

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(
		-Wall 
		-Wextra 
		-pedantic 
		-Werror 
		-Wno-unused-function 	# I want unused functions for debugging purposes
	)	
endif()
	
add_library(fake-socket STATIC ${C_FILES} ${H_FILES})
add_library(fake-socket::fake-socket ALIAS fake-socket)
	