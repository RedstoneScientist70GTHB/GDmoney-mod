cmake_minimum_required(VERSION 3.21)
project(MoneyMod VERSION 1.0.0)

# Assumes the Geode CLI / SDK is set up and CMAKE_TOOLCHAIN_FILE points at it,
# e.g. via `geode new` scaffolding or the standard Geode SDK toolchain file.

file(GLOB SOURCES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Set the GEODE_SDK environment variable, or install Geode CLI (https://docs.geode-sdk.org/getting-started/)")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
