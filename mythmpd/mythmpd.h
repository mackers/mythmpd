#ifndef MYTHMPD_H
#define MYTHMPD_H

#include <mythtv/uitypes.h>
#include <mythtv/uilistbtntype.h>
#include <mythtv/xmlparse.h>
#include <mythtv/mythdialogs.h>
#include <qtimer.h>

#include "libmpdclient.h"
#include "mythmpd.h"

class MythMPD : virtual public MythThemedDialog
{

Q_OBJECT

public:

    MythMPD(MythMainWindow *parent, QString windowName,
           QString themeFilename, const char *name = 0);
    ~MythMPD();

public slots:

    int updateStatus();
    void statusTimerDone();

private:

    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *e);
    int updatePlayList();

    void clearInfoBoxText();
    void clearPlayList();
    void setStatusText(char *text);
    void setStatusTextTimer(char *text);
    void setInfo(char *title, char *artist, char *album);
    void setElapsedTime(int elapsedSeconds, int totalSeconds);

    int connectMPD();
    void disconnectMPD();
    void reset();
    void dieMPD(char *error);

    char* secondsToMinutesAndSeconds(int seconds);

protected:

    mpd_Connection * conn;
    QTimer *timer;
    QTimer *statusTimer;
    UIListType *ltype;
    int isConnecting;
    int isPlaying;
    int isTicking;
    int isStopped;
    int volume;
    int previousVolume;
    int blockStatusTextUpdates;
    int songPosition;
    int previousSongPosition;
    int tracksInPlayList;
    int previousTracksInPlayList;

};


#endif
