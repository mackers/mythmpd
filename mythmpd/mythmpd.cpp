// POSIX headers
#include <unistd.h>

// MythTV headers
#include <mythuibutton.h>
#include <mythuibuttonlist.h>
#include <mythmainwindow.h>
#include <mythcontext.h>
#include <mythdirs.h>
#include <mythtv/mythdialogs.h>

// MythMPD headers
#include "mythmpd.h"

#define LOC      QString("MythMPD: ")
#define LOC_WARN QString("MythMPD, Warning: ")
#define LOC_ERR  QString("MythMPD, Error: ")

/** \brief Creates a new MythMPD Screen
*  \param parent Pointer to the screen stack
*  \param name The name of the window
*/
MythMPD::MythMPD(MythScreenStack *parent, QString name) :
         MythScreenType(parent, name),
         m_cancelButton(NULL)
{
    reset();
    //example of how to find the configuration dir currently used.
    QString confdir = GetConfDir();
    VERBOSE(VB_IMPORTANT, LOC + "Conf dir:"  + confdir);
}

bool MythMPD::Create()
{
    bool foundtheme = false;

    // Load the theme for this screen
    foundtheme = LoadWindowFromXML("mythmpd-ui.xml", "MythMPD", this);
    
    if (!foundtheme)
        return false;
    
    bool err = false;
    UIUtilE::Assign(this, m_fileText, "file", &err);
    UIUtilE::Assign(this, m_artistText, "artist", &err);
    UIUtilE::Assign(this, m_playtimeText, "time", &err);
    UIUtilE::Assign(this, m_statusText, "status", &err);
    UIUtilE::Assign(this, m_MPDinfoText, "MPDinfo", &err);
    UIUtilE::Assign(this, m_cancelButton, "cancel", &err);
    UIUtilE::Assign(this, m_mpdButtonList, "mpd_button_list", &err);
    
    if (err)
    {
        VERBOSE(VB_IMPORTANT, "Cannot load screen 'MythMPD'");
        return false;
    }
    
    BuildFocusList();
    clearPlayList();
    clearInformationText();
    updatePlayList();
    SetFocusWidget(m_mpdButtonList);

//    connect(m_connect_mpdButton, SIGNAL(Clicked()), this, SLOT(connect_mpd_clicked()));
//    connect(m_disconnect_mpdButton, SIGNAL(Clicked()), this, SLOT(disconnect_mpd_clicked()));
    connect(m_cancelButton, SIGNAL(Clicked()), this, SLOT(cancel_clicked()));
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    timer->start(1500);
    return true;
}

bool MythMPD::keyPressEvent(QKeyEvent *event)
{
    if (GetFocusWidget() && GetFocusWidget()->keyPressEvent(event))
        return true;
    
    bool handled = false;
    QStringList actions;
    handled = GetMythMainWindow()->TranslateKeyPress("Music", event, actions);

    for (unsigned int i = 0; i < actions.size() && !handled; i++)
    {   
        QString action = actions[i];
	handled = true;

	if (action == "PAUSE")
	{
	    if (isStopped)
	    {
	        mpd_sendPlayCommand(conn, 0);
	        mpd_finishCommand(conn);
	    }
	    else
	    {
	        mpd_sendPauseCommand(conn, isPlaying);
	        mpd_finishCommand(conn);
	    }
	}
	else if (action == "NEXTTRACK")
	{
	    mpd_sendNextCommand(conn);
	    mpd_finishCommand(conn);
	}
	else if (action == "PREVTRACK")
	{
	    mpd_sendPrevCommand(conn);
	    mpd_finishCommand(conn);
	}
	else if (action == "FFWD")
	{
	}
	else if (action == "RWND")
	{
	}
	else if (action == "STOP")
	{
            mpd_sendStopCommand(conn);
	    mpd_finishCommand(conn);
	}
	else if (action == "VOLUMEDOWN")
	{
	    volume -= 5;
	    if (volume < 0) volume = 0;

	    char buf[16];
	    sprintf(buf, "Volume: %d%%", volume);
            setStatus(buf);
	    
            mpd_sendSetvolCommand(conn, volume);
	    mpd_finishCommand(conn);
	}
	else if (action == "VOLUMEUP")
	{
	    volume += 5;
	    if (volume > 100) volume = 100;

	    char buf[16];
	    sprintf(buf, "Volume %d%%", volume);
            setStatus(buf);

	    mpd_sendSetvolCommand(conn, volume);
	    mpd_finishCommand(conn);
	}
	else if (action == "MUTE")
	{
	    if (previousVolume == 0)
	    {
	    	previousVolume = volume;
		volume = 0;
	    }
	    else
	    {
	    	volume = previousVolume;
		previousVolume = 0;
	    }

            mpd_sendSetvolCommand(conn, volume);
	    mpd_finishCommand(conn);
	}
	else if (action == "UP")
	{
	}
	else if (action == "DOWN")
	{
	}
	else if (action == "SELECT")
	{
	}
	else
	{
	    handled = false;
	}
    }
    
    if (!handled && MythScreenType::keyPressEvent(event))
        handled = true;

    return handled;
}


void MythMPD::setStatus(char *text)
{
    blockStatusTextUpdates = 1;
    setStatusText(text);
    statusTimer = new QTimer(this);
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusTimerDone()));
    statusTimer->start(1500);
}

void MythMPD::statusTimerDone()
{
    blockStatusTextUpdates = 0;
}

void MythMPD::setElapsedTime(int elapsedSeconds, int totalSeconds)
{
    char buf[200];
    char es[100];
    char ts[100];

    strcpy(es, secondsToMinutesAndSeconds(elapsedSeconds));
    strcpy(ts, secondsToMinutesAndSeconds(totalSeconds));

    sprintf(buf, "%s/%s", es, ts);
    m_playtimeText->SetText(QString(es));
}

char* MythMPD::secondsToMinutesAndSeconds(int seconds)
{
    float s = seconds;
    static char buf[100];
    int h, m;

    s = (int)(10. * s + .5) / 10.;
    h = (int)(s / 3600.);
    s -= 3600 * h;
    m = (int)(s / 60.);
    s -= 60 * m;
    int s2 = (int)s;
    sprintf(buf, "%02d:%02d", m, s2);
    return buf;
}


void MythMPD::updateStatus()
{
    if (conn == NULL)
    {
        if (!(connectMPD()))
        {
            dieMPD("Connection closed");
	    return;
        }
    }

    if (isTicking == 1)
    {
    	return;
    }

    //printf("Tick\n");

    mpd_Status * status;
    mpd_InfoEntity * entity;
    int doUpdatePlayList = 0;
    isTicking = 1;

    mpd_sendCommandListOkBegin(conn);

    if (conn->error)
    {
        isTicking = 0;
	dieMPD("Connection closed");
	return;
    }

    mpd_sendStatusCommand(conn);

    if (conn->error)
    {
        isTicking = 0;
	dieMPD("Connection closed");
	return;
    }

    mpd_sendCurrentSongCommand(conn);
    
    if (conn->error)
    {
        isTicking = 0;
	dieMPD("Connection closed");
	return;
    }

    mpd_sendCommandListEnd(conn);

    if (conn->error)
    {
        isTicking = 0;
	dieMPD("Connection closed");
	return;
    }

    if((status = mpd_getStatus(conn))==NULL)
    {
	dieMPD("Connection closed");
	isTicking = 0;
	return;
    }

    if (status->state == MPD_STATUS_STATE_PAUSE)
    {
	isPlaying = 0;
	isStopped = 0;

        setStatusText("Paused");
    }
    else if (status->state == MPD_STATUS_STATE_PLAY)
    {
	isPlaying = 1;
	isStopped = 0;

	if (volume == 0)
        {
            setStatusText("Muted");
	}
	else 
	{
            setStatusText("Playing");
	}
    }
    else
    {
	if (isStopped == 0)
	    doUpdatePlayList = 1;

	isStopped = 1;
	songPosition = -1;
	previousSongPosition = -2;

        setMPDinfoText("Connected");
	setPlayInfo("No file", "Nothing Playing");
	m_playtimeText->SetText(QString("--/--"));
    }

    songPosition = status->song;
    tracksInPlayList = status->playlistLength;
    volume = status->volume;

    // TODO update playlist if playlist id is different

    if (tracksInPlayList != previousTracksInPlayList)
    {
	previousTracksInPlayList = tracksInPlayList;
	doUpdatePlayList = 1;
    }

    if (status->state == MPD_STATUS_STATE_PLAY && status->elapsedTime < 5)
    {
    	doUpdatePlayList = 1;
    }

    if (status->state == MPD_STATUS_STATE_PAUSE || status->state == MPD_STATUS_STATE_PLAY)
    {
	if (songPosition != previousSongPosition)
	{
	      previousSongPosition = songPosition;
	      doUpdatePlayList = 1;
	}

        setElapsedTime(status->elapsedTime, status->totalTime);

	if (conn->error)
	{
		mpd_freeStatus(status);
		isTicking = 0;
		dieMPD("Connection closed");
		return;
	}

	if (doUpdatePlayList == 1)
	{
            mpd_nextListOkCommand(conn);

	    if (conn->error)
	    {
		mpd_freeStatus(status);
		isTicking = 0;
		dieMPD("Connection closed");
		return;
	    }

	    while ((entity = mpd_getNextInfoEntity(conn)))
	    {
		    mpd_Song * song = entity->info.song;

		    if (entity->type!=MPD_INFO_ENTITY_TYPE_SONG)
		    {
			    mpd_freeInfoEntity(entity);
			    continue;
		    }

		    setPlayInfo(song->artist, song->file);

		    mpd_freeInfoEntity(entity);
	    }

	    if (conn->error)
	    {
		    mpd_freeStatus(status);
		    isTicking = 0;
		    dieMPD("Connection closed");
		    return;
	    }
	}
    }

    mpd_finishCommand(conn);

    if (conn->error)
    {
	mpd_freeStatus(status);
	isTicking = 0;
	dieMPD("Connection closed");
	return;
    }

    mpd_freeStatus(status);

    if (doUpdatePlayList == 1)
        updatePlayList();

    isTicking = 0;

    return;
}



bool MythMPD::connectMPD()
{
    isConnecting = 1;
    conn = mpd_newConnection("localhost", 6600, 10);
    if (conn->error)
    {
	dieMPD("Could not connect");
        VERBOSE(VB_IMPORTANT, "MythMPD: Could not connect to mpd");
	isConnecting = 0;
	return false;
    }
    isConnecting = 0;

    return true;
}

void MythMPD::disconnectMPD()
{
    if (conn != NULL)	
	    mpd_closeConnection(conn);
    VERBOSE(VB_IMPORTANT, "MythMPD: mpd connection closed");
}

void MythMPD::dieMPD(char *error)
{
    //timer->stop();

    if (conn != NULL)
	    fprintf(stderr,"%s\n",conn->errorStr);

    setMPDinfoText(error);
    setPlayInfo("No file", "Nothing Playing");
    clearPlayList();

    disconnectMPD();
    reset();
}

void MythMPD::setMPDinfoText(char *text)
{
    m_MPDinfoText->SetText(QString(text));
}

void MythMPD::setStatusText(char *text)
{
    if (blockStatusTextUpdates == 0)
        m_statusText->SetText(QString(text));
}

void MythMPD::setPlayInfo(char *artist, char *file)
{
    if (not(artist)) {artist = "unknown";}
    m_fileText->SetText(QString(file));
    m_artistText->SetText(QString(artist));
}


void MythMPD::clearInformationText()
{
    m_fileText->SetText(QString("--"));
    m_artistText->SetText(QString("--"));
    m_playtimeText->SetText(QString("--"));
    m_MPDinfoText->SetText(QString("--"));
}

void MythMPD::cancel_clicked(void)
{
   setStatusText("cancel clicked");
}

void MythMPD::connect_mpd_clicked(void)
{
    if (connectMPD())
        VERBOSE(VB_IMPORTANT, "MythMPD: connected to mpd");
}

void MythMPD::disconnect_mpd_clicked(void)
{
  //  m_statusText->SetText(QString("disconnected from MPD"));
}


void MythMPD::clearPlayList()
{
    m_mpdButtonList->Reset();
}

void MythMPD::reset()
{
    conn = NULL;
    isConnecting = 0;
    isPlaying = 0;
    isStopped = 0;
    isTicking = 0;
    blockStatusTextUpdates = 0;
    volume = 80;
    previousVolume = 0;
    songPosition = -1;
    previousSongPosition = -2;
    tracksInPlayList = 1;
    previousTracksInPlayList = -1;
    VERBOSE(VB_IMPORTANT, "MythMPD: Playlist cleared/reset");
}

int MythMPD::updatePlayList()
{
    //printf("Updating playlist...\n");

    mpd_InfoEntity *entity;
    if (conn == NULL)
    {
        if (!(connectMPD()))
        {
            dieMPD("Connection closed");
	    return -1;
        }
        //connect_mpd_clicked();
    }
    setMPDinfoText("Connected");
    mpd_sendPlaylistInfoCommand(conn,-1);

    int startWindow = songPosition - 6;
    if (startWindow < 0) startWindow = 0;
    int endWindow = startWindow + 13;

    int counter = 0;
    m_mpdButtonList->Reset();
    entity = mpd_getNextInfoEntity(conn);

    VERBOSE(VB_IMPORTANT, "MythMPD: preparing playlist");

    while (entity)
    {
        if (entity->type != MPD_INFO_ENTITY_TYPE_SONG)
        {
            mpd_freeInfoEntity(entity);
	    continue;
        };

        mpd_Song *song = entity->info.song;
        int thisPosition = song->pos;
        counter++;
        char buf[256];
        if (not(song->title))
        {
            sprintf(buf, "%s / %s / %s", song->file, song->artist, song->album);
        }
        else
        {
            sprintf(buf, "%s / %s / %s", song->title, song->artist, song->album);
        }

        VERBOSE(VB_IMPORTANT, buf);

        if (thisPosition >= startWindow && thisPosition <= endWindow)
        {
            MythUIButtonListItem *mpdItem = new MythUIButtonListItem(m_mpdButtonList,QString(buf));

            if (thisPosition == songPosition && !isStopped)
            {
	    };

	};

        mpd_freeInfoEntity(entity);
        entity = mpd_getNextInfoEntity(conn);
    };

    if (isStopped)
    {
       // m_mpdButtonList->SetItemCurrent(-1);
    };

    if (conn->error)
    {
        dieMPD("Connection closed");
	return -1;
    }

    mpd_finishCommand(conn); 

    if (conn->error)
    {
        dieMPD("Connection closed");
	return -1;
    }
    return 0;
}

