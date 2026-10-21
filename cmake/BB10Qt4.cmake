# cmake/BB10Qt4.cmake
# Minimal stub for BB10 Qt4 CMake module
# This module defines the QNX_TARGET variable and sets up basic Qt4 include/link directories.

# You may need to adjust paths if your SDK is elsewhere.
set(QNX_TARGET "/opt/qnx/6.5.0") # replace with actual SDK root

# Tell CMake where to find Qt headers and libs
set(QNX_QT4_INCLUDE_DIR "$${QNX_TARGET}/usr/include/qt4")
set(QNX_QT4_LIBRARY_DIR "$${QNX_TARGET}/usr/lib/$${CMAKE_LIBRARY_ARCHITECTURE}")

# Export variables for the rest of the project
add_definitions(-D__QNXNTO__)

# Provide Qt4 packages via find_package (Qt4 is not a real find module here, but this works in our limited context)
message(STATUS "Using BBNDK at $${QNX_TARGET}")
