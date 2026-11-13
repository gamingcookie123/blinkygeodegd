cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(Blinky VERSION 1.0.0)

# Download Geode SDK if not found
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable.")
else()
    message(STATUS "Using Geode SDK at $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/ui/*.cpp
    src/game/*.cpp
    src/levels/*.cpp
    src/util/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    src/ui/
    src/game/
    src/levels/
    src/util/
)

setup_geode_mod(${PROJECT_NAME})
