cmake_minimum_required (VERSION 3.19)

file(GLOB_RECURSE C_FILES "*.c")
file(GLOB_RECURSE H_FILES "*.h")

add_executable(fake-socket-test ${C_FILES} ${H_FILES})
	
target_link_libraries(fake-socket-test fake-socket::fake-socket)

if(NOT MSVC)
	target_link_libraries(fake-socket-test pthread)
endif()

if(WIN32)
	target_link_libraries(fake-socket-test ws2_32)
else()
	target_link_libraries(fake-socket-test pthread)
endif()


add_test(fake-socket-test fake-socket-test)