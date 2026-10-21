cmake_minimum_required(VERSION 3.20)
project(compas_probes LANGUAGES CXX)

# BB10 expects ARM binaries in this directory structure
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")

# Force C++11 (bb10-c++ defaults to gnu++98)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # gnu++11

# Architecture define for ProbeRunner
add_definitions(-DQT_ARCH=\"armv7-softfp\")

# Qt4 moc/rcc/uic support (provided by XIAZY toolchain)
include(BB10Qt4)
qt4_wrap_cpp(MOC_SOURCES ProbeRunner.h)
qt4_add_resources(QRC_SOURCES assets/qml.qrc)

# CMake's Generic platform declares that it cannot build shared libraries
# (Platform/Generic.cmake sets TARGET_SUPPORTS_SHARED_LIBS to FALSE), but the
# BB10 toolchain can: BB10LinkRules.cmake defines CMAKE_<LANG>_CREATE_SHARED_LIBRARY
# and bb10-c++ passes -shared straight through to the QNX linker.
# Override the platform flag so add_library(... SHARED ...) works.
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

# ---------------------------------------------------------------------------
# The probe runner: the BAR entry point.  Must be an ELF binary -- a shell
# script is not a valid BB10 entry point.
# ---------------------------------------------------------------------------
add_executable(compas-probes
    probe_runner.cpp
    ProbeRunner.cpp
    ${MOC_SOURCES}
    ${QRC_SOURCES}
)

target_link_libraries(compas-probes PRIVATE
    bbcascades
    QtDeclarative
    QtCore
    QtGui
)

# ---------------------------------------------------------------------------
# The individual probe executables.  These are launched by the runner at
# runtime, not linked into it.
# ---------------------------------------------------------------------------
set(PROBE_SOURCES
    filesystem_probe
    posix_probe
    thread_probe
    loader_probe
)

foreach(probe ${PROBE_SOURCES})
    add_executable(${probe} ${probe}.cpp)
    set_target_properties(${probe} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes"
    )
    target_link_libraries(${probe} PRIVATE QtCore)
endforeach()

# The plugin shared library used by loader_probe to exercise dlopen/dlsym.
# The target is named "probe" so that CMake produces "libprobe.so"
# (the "lib" prefix and ".so" suffix are added automatically).
add_library(probe SHARED plugin.cpp)
set_target_properties(probe PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes"
)

# ---------------------------------------------------------------------------
# Common settings
# ---------------------------------------------------------------------------
foreach(target compas-probes ${PROBE_SOURCES})
    target_include_directories(${target} PRIVATE
        ${QNX_TARGET}/usr/include/qt4
        ${QNX_TARGET}/usr/include/qt4/QtCore
        ${QNX_TARGET}/usr/include/qt4/QtGui
        ${QNX_TARGET}/usr/include/qt4/QtNetwork
        ${QNX_TARGET}/usr/include/qt4/QtSql
        ${QNX_TARGET}/usr/include/qt4/QtDeclarative
    )
endforeach()

# Override compiler flags to ensure C++11 is used
set_target_properties(compas-probes ${PROBE_SOURCES} probe PROPERTIES
    COMPILE_FLAGS "-std=gnu++11 -fexceptions -frtti -fno-use-cxa-atexit"
)

# Ensure executable name matches bar-descriptor.xml
set_target_properties(compas-probes PROPERTIES OUTPUT_NAME "compas-probes")

# ---------------------------------------------------------------------------
# Stage the BAR assets
#
# The BAR descriptor lists assets by their path relative to the project
# directory.  The probe binaries and the private runtime libraries therefore
# have to be copied into assets/ before packaging.
# ---------------------------------------------------------------------------
set(STAGE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(STAGE_PROBES_DIR "${STAGE_DIR}/probes")
set(STAGE_LIBS_DIR "${STAGE_DIR}/libs")

# The SDK's libstdc++.so.6 always matches the compiler that produced the
# binaries, so it is copied at build time rather than committed.
# QNX_TARGET is an environment variable set by the toolchain image; fall back
# to the image's default sysroot if it is not exported.
if(DEFINED ENV{QNX_TARGET})
    set(SDK_TARGET "$ENV{QNX_TARGET}")
else()
    set(SDK_TARGET "/opt/bb10-sdk/target/qnx6")
endif()
set(SDK_LIBSTDCXX "${SDK_TARGET}/armle-v7/lib/libstdc++.so.6")

add_custom_target(stage-assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${STAGE_PROBES_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${STAGE_LIBS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes/filesystem_probe" "${STAGE_PROBES_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes/posix_probe" "${STAGE_PROBES_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes/thread_probe" "${STAGE_PROBES_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes/loader_probe" "${STAGE_PROBES_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes/libprobe.so" "${STAGE_PROBES_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDK_LIBSTDCXX}" "${STAGE_LIBS_DIR}/"
    DEPENDS compas-probes ${PROBE_SOURCES} probe
    COMMENT "Staging BAR assets"
    VERBATIM
)
add_dependencies(stage-assets compas-probes ${PROBE_SOURCES} probe)
