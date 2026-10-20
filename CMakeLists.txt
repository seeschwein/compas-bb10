cmake_minimum_required(VERSION 3.20)
project(compas LANGUAGES CXX)

# BB10 expects ARM binaries in this directory structure
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")

# Qt4 moc/rcc/uic support (provided by XIAZY toolchain)
include(BB10Qt4)
qt4_wrap_cpp(MOC_SOURCES src/TileCache.h)
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

# Ensure executable name matches bar-descriptor.xml
set_target_properties(compas PROPERTIES OUTPUT_NAME "compas")