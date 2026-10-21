cmake_minimum_required(VERSION 3.20)
project(compas_probes LANGUAGES CXX)

# BB10 expects ARM binaries in this directory structure
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")

# Force C++11 (bb10-c++ defaults to gnu++98)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # gnu++11

# Qt4 moc/rcc/uic support (provided by XIAZY toolchain)
include(BB10Qt4)
qt4_add_resources(QRC_SOURCES assets/qml.qrc)

# ---------------------------------------------------------------------------
# The probe runner: the BAR entry point.  Must be an ELF binary -- a shell
# script is not a valid BB10 entry point.
# ---------------------------------------------------------------------------
add_executable(compas-probes
    probe_runner.cpp
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
add_library(probe SHARED plugin.cpp)
set_target_properties(probe PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g/probes"
    OUTPUT_NAME "libprobe"
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
