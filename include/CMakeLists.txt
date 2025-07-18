cmake_minimum_required(VERSION 3.19)

target_include_directories(fake-socket PUBLIC  
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

file(GLOB H_FILES "fs/*.h")

target_sources(fake-socket PRIVATE ${H_FILES})

install(DIRECTORY fake-socket DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
