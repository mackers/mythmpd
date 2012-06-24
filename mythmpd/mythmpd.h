#ifndef MYTHMPD_H
#define MYTHMPD_H

// MythTV headers
#include <QObject>
#include <mythscreentype.h>
#include <mythuibutton.h>
#include <mythuibuttonlist.h>
#include <mythtv/uitypes.h>
#include <mythuitext.h>
#include <mythdialogbox.h>
#include "libmpdclient.h"

extern mpd_Connection *conn;

/** \class MythMPD */
class MythMPD : public MythScreenType
{
    Q_OBJECT
    
    public:
        MythMPD(MythScreenStack *parentStack
               , unsigned int *executed
               , QString name = "MythMPD");
        bool Create(void);
        bool keyPressEvent(QKeyEvent *event);
        //mpd_Connection * getMPDConnection();

    public slots:
        void statusTimerDone();
        void updateStatus();
        
    private:
        unsigned int   *m_executed;
        MythUIText *m_textTitle;
        MythUIText *m_textAlbum;
        MythUIText *m_textArtist;
        MythUIText *m_playtimeText;
        MythUIText *m_statusText;
        MythUIText *m_MPDinfoText;
        MythUIButton   *m_buttonPlayQueue;
        MythUIButton   *m_buttonDatabase;
        MythUIButton   *m_buttonPlaylistEditor;
        bool connectMPD();
        void clearInformationText();
        void setPlayInfo(char*, char*);
        void setMPDinfoText(char*);
        void clearPlayList();
        void setStatus(char *text);
        void setStatusText(char *text);
        int updatePlayList();
        void disconnectMPD();
        void reset();
        void dieMPD(char *error);
        void setElapsedTime(int elapsedSeconds, int totalSeconds);
        char* secondsToMinutesAndSeconds(int seconds);

    private slots:
        void clicked_PlayQueue(void);
        void clicked_Database(void);
        void clicked_PlaylistEditor(void);

    protected:
        QTimer *timer;
        QTimer *statusTimer;
        //UIListType *ltype;
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

#endif /* MYTHMPD_H */
