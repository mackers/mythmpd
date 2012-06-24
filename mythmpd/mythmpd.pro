include ( ../../mythconfig.mak )
include ( ../../settings.pro )
include ( ../../programs-libs.pro )

QT += network sql xml core gui uitools

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
HEADERS += mythmpd.h libmpdclient.h
HEADERS += mythmpd_playqueue.h
SOURCES += main.cpp mythmpd.cpp libmpdclient.c
SOURCES += mythmpd_playqueue.cpp 

macx {
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
}
