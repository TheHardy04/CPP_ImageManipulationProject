# Minimum version of CMake
cmake_minimum_required(VERSION 3.16)

# Project name and C++ standard
project(ImageProject)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set source and include directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB SOURCES ${SRC_DIR}/*.cpp)
include_directories(${SRC_DIR})

# Copy Images directory to build folder
file(COPY ${CMAKE_SOURCE_DIR}/Images DESTINATION ${CMAKE_BINARY_DIR})

# Define source and destination paths
set(SOURCE_IMAGES_DIR ${CMAKE_SOURCE_DIR}/Images)
set(BINARY_IMAGES_DIR ${CMAKE_BINARY_DIR}/Images)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

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

# Copy the Images folder after the build
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${BINARY_IMAGES_DIR} ${SOURCE_IMAGES_DIR}
    COMMENT "Copying Images folder from binary to source directory..."
)
