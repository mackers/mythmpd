#ifndef MYTHMPD_H
#define MYTHMPD_H

// MythTV headers

#include <mythscreentype.h>
#include <mythuibutton.h>
#include <mythuibuttonlist.h>
#include <mythtv/uitypes.h>
#include <mythuitext.h>
#include "libmpdclient.h"

/** \class MythMPD
*  \brief Example plugin.  Shows how to use Text areas and buttons
*/
class MythMPD : public MythScreenType
{
    Q_OBJECT
    
    public:
        MythMPD(MythScreenStack *parent, QString name);
        //~MythMPD();
        bool Create();
        bool keyPressEvent(QKeyEvent *event);

    public slots:
        void statusTimerDone();
        void updateStatus();
        
    private:
        MythUIText *m_fileText;
        MythUIText *m_artistText;
        MythUIText *m_playtimeText;
        MythUIText *m_statusText;
        MythUIText *m_MPDinfoText;
        MythUIButton   *m_disconnect_mpdButton;
        MythUIButton   *m_connect_mpdButton;
        MythUIButton   *m_cancelButton;
        MythUIButtonList *m_mpdButtonList;
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
        void disconnect_mpd_clicked(void);
        void connect_mpd_clicked(void);
        void cancel_clicked(void);

    protected:
         mpd_Connection *conn;
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

#endif /* MYTHMPD_H */
