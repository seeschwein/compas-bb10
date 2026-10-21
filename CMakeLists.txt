cmake_minimum_required(VERSION 3.20)
project(compas LANGUAGES CXX)

# BB10 expects ARM binaries in this directory structure
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")

# Force C++11 (bb10-c++ defaults to gnu++98)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # gnu++11

# Qt4 moc/rcc/uic support.  BB10Qt4.cmake ships with the XIAZY toolchain
# (in /opt/bb10-cmake, which the toolchain file adds to CMAKE_MODULE_PATH).
include(BB10Qt4)
qt4_wrap_cpp(MOC_SOURCES src/TileCache.h src/TileNetwork.h)
qt4_add_resources(QRC_SOURCES assets/qml.qrc)

# QNX_TARGET is an environment variable set by the toolchain image; fall back
# to the image's default sysroot if it is not exported.
if(DEFINED ENV{QNX_TARGET})
    set(SDK_TARGET "$ENV{QNX_TARGET}")
else()
    set(SDK_TARGET "/opt/bb10-sdk/target/qnx6")
endif()

add_executable(compas
    src/main.cpp
    src/TileCache.cpp
    src/TileNetwork.cpp
    ${MOC_SOURCES}
    ${QRC_SOURCES}
)

target_link_libraries(compas PRIVATE
    bbcascades
    QtDeclarative
    QtCore
    QtGui
    QtNetwork
    QtSql
)

# Qt4 on BB10 requires explicit include paths for Qt modules
target_include_directories(compas PRIVATE
    ${SDK_TARGET}/usr/include/qt4
    ${SDK_TARGET}/usr/include/qt4/QtCore
    ${SDK_TARGET}/usr/include/qt4/QtGui
    ${SDK_TARGET}/usr/include/qt4/QtNetwork
    ${SDK_TARGET}/usr/include/qt4/QtSql
    ${SDK_TARGET}/usr/include/qt4/QtDeclarative
)

# Ensure executable name matches bar-descriptor.xml
set_target_properties(compas PROPERTIES OUTPUT_NAME "compas")

# Override compiler flags to ensure C++11 is used
set_target_properties(compas PROPERTIES
    COMPILE_FLAGS "-std=gnu++11 -fexceptions -frtti -fno-use-cxa-atexit"
)

# ---------------------------------------------------------------------------
# Stage the BAR assets
#
# The BAR descriptor lists assets by their path relative to the project
# directory, so the private runtime libraries have to be copied into
# assets/libs/ before packaging.
# ---------------------------------------------------------------------------
set(STAGE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(STAGE_LIBS_DIR "${STAGE_DIR}/libs")

# The SDK's libstdc++.so.6 always matches the compiler that produced the
# binaries, so it is copied at build time rather than committed.
set(SDK_LIBSTDCXX "${SDK_TARGET}/armle-v7/lib/libstdc++.so.6")

# libgcc_s.so.1 is not in the SDK; the repo keeps a copy in assets/libs/
add_custom_target(stage-assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${STAGE_LIBS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDK_LIBSTDCXX}" "${STAGE_LIBS_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/libs/libgcc_s.so.1" "${STAGE_LIBS_DIR}/"
    DEPENDS compas
    COMMENT "Staging BAR assets"
    VERBATIM
)
add_dependencies(stage-assets compas)
