cmake_minimum_required(VERSION 3.20)
project(compas-native LANGUAGES CXX)

# Build for ARMv7 BB10 using bb10-cxx20 toolchain
set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_SYSTEM_VERSION 6.6.0)
set(CMAKE_SYSTEM_PROCESSOR armv7)

# Use the bb10-cxx20 toolchain file if available
if(EXISTS "/opt/bb10-cxx20/share/bb10-cxx20.cmake")
    set(CMAKE_TOOLCHAIN_FILE "/opt/bb10-cxx20/share/bb10-cxx20.cmake" CACHE FILEPATH "" FORCE)
endif()

# Output to BB10 standard layout
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # gnu++20

# Target: shared library libcompas_native.so
add_library(compas_native SHARED compas_native.cpp)

# Link with system libraries available in BB10 SDK
target_link_libraries(compas_native PRIVATE
    curl
    sqlite3
    z
    fmt
)

# Include paths
target_include_directories(compas_native PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    /opt/bb10-sdk/target/qnx6/armle-v7/usr/include
)

# Compile options
target_compile_options(compas_native PRIVATE
    -fexceptions
    -frtti
    -fvisibility=hidden
    -Wall
    -Wextra
)

# Output name matches what bar-descriptor.xml expects
set_target_properties(compas_native PROPERTIES OUTPUT_NAME "compas_native")