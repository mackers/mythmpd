// POSIX headers
#include <unistd.h>

// MythTV headers
#include <mythuibutton.h>
#include <mythuibuttonlist.h>
#include <mythmainwindow.h>
#include <mythcontext.h>
#include <mythdirs.h>
#include <mythtv/mythdialogs.h>
#include <QSettings>

// MythMPD headers
#include "libmpdclient.h"
#include "mythmpd.h"
#include "mythmpd_playqueue.h"

#define LOC      QString("MythMPD: ")
#define LOC_WARN QString("MythMPD, Warning: ")
#define LOC_ERR  QString("MythMPD, Error: ")

/** \brief Creates a new MythMPD Screen
*  \param parent Pointer to the screen stack
*  \param name The name of the window
*/
MythMPD::MythMPD(MythScreenStack *parent
                , unsigned int *executed
                , QString name) :
         MythScreenType(parent, name)
                , m_buttonPlayQueue(NULL)
                , m_buttonDatabase(NULL)
                , m_buttonPlaylistEditor(NULL)
{
    m_executed = executed;
}


bool MythMPD::Create()
{
    bool foundtheme = false;

    // Load the theme for this screen
    foundtheme = LoadWindowFromXML("mythmpd-ui.xml", "MythMPD", this);
    
    if (!foundtheme)
        return false;

    reset();
    bool err = false;
    UIUtilE::Assign(this, m_textTitle,            "text_Title"           , &err);
    UIUtilE::Assign(this, m_textAlbum,            "text_Album"           , &err);
    UIUtilE::Assign(this, m_textArtist,           "text_Artist"          , &err);
    UIUtilE::Assign(this, m_playtimeText,         "text_Time"            , &err);
    UIUtilE::Assign(this, m_statusText,           "status"               , &err);
    UIUtilE::Assign(this, m_MPDinfoText,          "MPDinfo"              , &err);
    UIUtilE::Assign(this, m_buttonPlayQueue,      "button_PlayQueue"     , &err);
    UIUtilE::Assign(this, m_buttonDatabase,       "button_Database"      , &err);
    UIUtilE::Assign(this, m_buttonPlaylistEditor, "button_PlaylistEditor", &err);
    
    if (err)
    {
        VERBOSE(VB_IMPORTANT, "Cannot load screen 'MythMPD'");
        return false;
    }
    
    BuildFocusList();
    clearPlayList();
    clearInformationText();
    updatePlayList();

    connect(m_buttonPlayQueue, SIGNAL(Clicked()), this, SLOT(clicked_PlayQueue()));
    connect(m_buttonDatabase, SIGNAL(Clicked()), this, SLOT(clicked_Database()));
    connect(m_buttonPlaylistEditor, SIGNAL(Clicked()), this, SLOT(clicked_PlaylistEditor()));

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

		    setPlayInfo(song->artist, song->title);

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
    //example of how to find the configuration dir currently used.
    QString m_sSettingsFile = GetConfDir() + "/mythmpd.conf";
    VERBOSE(VB_IMPORTANT, LOC + "Conf file:"  + m_sSettingsFile);

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    QString m_server_string = settings.value("server", "localhost").toString();
    QString m_port_string = settings.value("port", "6600").toString();
    QString m_timeout_seconds_string = settings.value("timeout", "10").toString();

    QByteArray ba = m_server_string.toLocal8Bit();
    const char *m_server = ba.data();
    int m_port = m_port_string.toInt();
    float m_timeout_seconds = m_timeout_seconds_string.toFloat();

    VERBOSE(VB_IMPORTANT, LOC + "Using " + m_server + ":" + m_port_string + " with " + m_timeout_seconds_string + " second timeout");    
    conn = mpd_newConnection(m_server, m_port, m_timeout_seconds);
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
    timer->stop();

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

void MythMPD::setPlayInfo(char *artist, char *title)
{
    if (not(artist)) {artist = "unknown";}
    m_textTitle->SetText(QString(title));
    m_textArtist->SetText(QString(artist));
}


void MythMPD::clearInformationText()
{
    m_textTitle->SetText(QString("--"));
    m_textArtist->SetText(QString("--"));
    m_playtimeText->SetText(QString("--"));
    m_MPDinfoText->SetText(QString("--"));
}


void MythMPD::clicked_PlayQueue(void)
{
    VERBOSE(VB_IMPORTANT, "MythMPD: clicked PlayQueue");
    int item;
    if (conn == NULL)
    {
        VERBOSE(VB_IMPORTANT, "MythMPD: Not connected");
    }
    else
    {
        MythScreenStack *mainStack = GetMythMainWindow()->GetMainStack();
        MythMPD_PlayQueue *PlayQueueScreen = new MythMPD_PlayQueue(mainStack,this,"PlayQueue");
        if (PlayQueueScreen->Create()) 
        {
            mainStack->AddScreen(PlayQueueScreen);
        }
        else
        {
            delete PlayQueueScreen;
        }
    }
}


void MythMPD::clicked_Database(void)
{
    VERBOSE(VB_IMPORTANT, "MythMPD: clicked Database");
}


void MythMPD::clicked_PlaylistEditor(void)
{
    VERBOSE(VB_IMPORTANT, "MythMPD: clicked PlaylistEditor");
}


void MythMPD::clearPlayList()
{

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
    mpd_InfoEntity *entity;
    if (conn == NULL)
    {
        if (!(connectMPD()))
        {
            dieMPD("Connection closed");
	    return -1;
        }
    }
    setMPDinfoText("Connected");
    mpd_sendPlaylistInfoCommand(conn,-1);

    int startWindow = songPosition - 6;
    if (startWindow < 0) startWindow = 0;
    int endWindow = startWindow + 13;

    int counter = 0;

    entity = mpd_getNextInfoEntity(conn);

    VERBOSE(VB_IMPORTANT, "MythMPD: update playlist");

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


