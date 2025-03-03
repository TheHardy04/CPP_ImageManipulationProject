# Minimum version of CMake
cmake_minimum_required(VERSION 3.16)

# Project name and C++ standard
project(ImageProject)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set source and include directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/Src)
file(GLOB SOURCES ${SRC_DIR}/*.cpp)
include_directories(${SRC_DIR})

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Reset Images directory in build folder
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Images
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Images ${CMAKE_BINARY_DIR}/Images
)

# Platform-specific settings (optional)
if(WIN32)
    message("Configuring for Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WINDOWS)
elseif(APPLE)
    message("Configuring for macOS")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_MACOS)
elseif(UNIX)
    message("Configuring for Linux")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_LINUX)
endif()