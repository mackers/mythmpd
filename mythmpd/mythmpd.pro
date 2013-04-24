QT += network sql xml core gui

INCLUDEPATH += /usr/include/mythtv/libmythui/
INCLUDEPATH += /usr/include/mythtv/

TEMPLATE = lib
CONFIG += plugin thread
TARGET = mythmpd
target.path = $${LIBDIR}/mythtv/plugins
INSTALLS += target

uifiles.path = $${PREFIX}/share/mythtv/themes/default
uifiles.files = mythmpd-ui.xml
installfiles.path = $${PREFIX}/share/mythtv
installfiles.files = mythmpd-ui.xml

INSTALLS += uifiles

# Input
HEADERS += mythmpd.h
SOURCES += main.cpp mythmpd.cpp

macx {
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
}

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libmpdclient
}
