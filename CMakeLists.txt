cmake_minimum_required(VERSION 3.20)
project(compas LANGUAGES CXX)

# BB10 expects ARM binaries in this directory structure
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")

# Force C++11 (bb10-c++ defaults to gnu++98)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # gnu++11

# Qt4 moc/rcc/uic support (provided by XIAZY toolchain)
include(BB10Qt4)
qt4_wrap_cpp(MOC_SOURCES src/TileCache.h src/TileNetwork.h)
qt4_add_resources(QRC_SOURCES assets/qml.qrc)

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
    ${QNX_TARGET}/usr/include/qt4
    ${QNX_TARGET}/usr/include/qt4/QtCore
    ${QNX_TARGET}/usr/include/qt4/QtGui
    ${QNX_TARGET}/usr/include/qt4/QtNetwork
    ${QNX_TARGET}/usr/include/qt4/QtSql
    ${QNX_TARGET}/usr/include/qt4/QtDeclarative
)

# Ensure executable name matches bar-descriptor.xml
set_target_properties(compas PROPERTIES OUTPUT_NAME "compas")

# Override compiler flags to ensure C++11 is used
set_target_properties(compas PROPERTIES
    COMPILE_FLAGS "-std=gnu++11 -fexceptions -frtti -fno-use-cxa-atexit"
)