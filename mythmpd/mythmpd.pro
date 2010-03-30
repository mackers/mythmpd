include ( ../../mythconfig.mak )
include ( ../../settings.pro )

TEMPLATE = lib
CONFIG += plugin thread
TARGET = mythmpd
target.path = $${LIBDIR}/mythtv/plugins
INSTALLS += target

uifiles.path = $${PREFIX}/share/mythtv/themes/default
uifiles.files = mpd-ui.xml
installfiles.path = $${PREFIX}/share/mythtv
installfiles.files = mpd-ui.xml

INSTALLS += uifiles

# Input
HEADERS += mythmpd.h libmpdclient.h
SOURCES += main.cpp mythmpd.cpp libmpdclient.c 

macx {
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
}
