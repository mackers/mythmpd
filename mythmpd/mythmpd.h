#ifndef MYTHMPD_H
#define MYTHMPD_H

// POSIX headers
#include <unistd.h>
#include <assert.h>

// MythTV headers
#include <QObject>
#include <QSettings>
#include <mythscreentype.h>
#include <mythuibutton.h>
#include <mythuibuttonlist.h>
#include <mythuitext.h>
#include <mythdialogbox.h>
#include <mythmainwindow.h>
#include <mythcontext.h>
#include <mythdirs.h>
#include <mythtv/uitypes.h>
#include <mythtv/mythdialogs.h>
#include <mpd/client.h>

#define QUEUE     0
#define PLAYLISTS 1
#define GENRES    2
#define ARTISTS   3
#define ALBUMS    4

extern mpd_connection *conn;

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
        MythUIButton   *m_buttonArtists;
        MythUIButton   *m_buttonPlaylists;
        MythUIButtonList   *m_buttonlistPlayQueue;
        bool connectMPD();
        void clearInformationText();
        void setPlayInfo(char*, char*, char*);
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
        void printErrorAndExit(struct mpd_connection *conn);
        void my_finishCommand(struct mpd_connection *conn);
        const char * song_value(const struct mpd_song *song, const char *name);
        bool update_playlist(bool doUpdatePlayList);

    private slots:
        void clicked_PlayQueue(void);
        void clicked_Playlists(void);
        void clicked_Artists(void);
        void clicked_track(void);


    protected:
        QTimer *timer;
        QTimer *statusTimer;
        //UIListType *ltype;
        int isConnecting;
        int isTicking;
        int volume;
        int previousVolume;
        int blockStatusTextUpdates;
        int songPosition;
        int previousSongPosition;
        int tracksInPlayList;
        int previousTracksInPlayList;
        int buttonlist_mode;
        int play_state;
};

#endif /* MYTHMPD_H */
