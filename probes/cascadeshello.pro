APP_NAME = compas-probes
CONFIG += qt warn_on cascades10
QT += core declarative sql network
SOURCES += src/main.cpp \
           src/TileCache.cpp \
           src/TileNetwork.cpp
HEADERS += src/TileCache.h \
           src/TileNetwork.h
OTHER_FILES += assets/main.qml bar-descriptor.xml assets/map/MapView.qml