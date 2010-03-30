#ifndef MYTHMPD_CPP
#define MYTHMPD_CPP

/* QT includes */
#include <qnamespace.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qbuttongroup.h>

/* MythTV includes */
#include <mythtv/mythcontext.h>
#include <mythtv/mythdialogs.h>

/* MPD includes */
#include "libmpdclient.h"

using namespace std;

#include "mythmpd.h"

MythMPD::MythMPD(MythMainWindow *parent, QString windowName,
                           QString themeFilename, const char *name)
        : MythThemedDialog(parent, windowName, themeFilename, name)
{
    reset();

    ltype = (UIListType *)getContainer("listbox")->GetType("playlist");

    clearInfoBoxText();
    clearPlayList();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    timer->start(1000); 
    statusTimer = NULL;
}

void MythMPD::clearInfoBoxText()
{
    getUITextType("track")->SetText("--");
    getUITextType("artist")->SetText("--");
    getUITextType("album")->SetText("--");
    getUITextType("time")->SetText("--/--");
}

void MythMPD::setStatusText(char *text)
{
    if (blockStatusTextUpdates == 0)
        getUITextType("status")->SetText(text);
}

void MythMPD::setStatusTextTimer(char *text)
{
    blockStatusTextUpdates = 1;
    getUITextType("status")->SetText(text);

    statusTimer = new QTimer(this);
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusTimerDone()));
    statusTimer->start(1500, TRUE);
}

void MythMPD::statusTimerDone()
{
    blockStatusTextUpdates = 0;
}

void MythMPD::clearPlayList()
{
    if (ltype != NULL)
    {   
        ltype->ResetList();
        ltype->SetActive(true);
    }

    updateForeground();
}

int MythMPD::connectMPD()
{
    isConnecting = 1;

    conn = mpd_newConnection("localhost", 6600, 10);

    if (conn->error)
    {
	dieMPD("Could not connect");
	isConnecting = 0;
	return -1;
    }

    isConnecting = 0;

    return 0;
}

void MythMPD::disconnectMPD()
{
    if (conn != NULL)
	    mpd_closeConnection(conn);
}

void MythMPD::dieMPD(char *error)
{
    //timer->stop();

    if (conn != NULL)
	    fprintf(stderr,"%s\n",conn->errorStr);

    setStatusText(error);
    clearInfoBoxText();
    clearPlayList();

    disconnectMPD();
    reset();
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
}

int MythMPD::updateStatus()
{
    if (conn == NULL)
    {
	if (isConnecting == 0)
		connectMPD();

	return -1;
    }

    if (isTicking == 1)
    {
    	return -1;
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
	return -1;
    }

    mpd_sendStatusCommand(conn);

    if (conn->error)
    {
        isTicking = 0;
	dieMPD("Connection closed");
	return -1;
    }

    mpd_sendCurrentSongCommand(conn);
    
    if (conn->error)
    {
        isTicking = 0;
	dieMPD("Connection closed");
	return -1;
    }

    mpd_sendCommandListEnd(conn);

    if (conn->error)
    {
        isTicking = 0;
	dieMPD("Connection closed");
	return -1;
    }

    if((status = mpd_getStatus(conn))==NULL)
    {
	//dieMPD("Connection closed");
	isTicking = 0;
	return -1;
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
            setStatusText("Connected");
	}
    }
    else
    {
	if (isStopped == 0)
	    doUpdatePlayList = 1;

	isStopped = 1;
	songPosition = -1;
	previousSongPosition = -2;

        setStatusText("Connected");
	setInfo("Nothing Playing", "", "");
	getUITextType("time")->SetText("--/--");
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
		return -1;
	}

	if (doUpdatePlayList == 1)
	{
            mpd_nextListOkCommand(conn);

	    if (conn->error)
	    {
		mpd_freeStatus(status);
		isTicking = 0;
		dieMPD("Connection closed");
		return -1;
	    }

	    while ((entity = mpd_getNextInfoEntity(conn)))
	    {
		    mpd_Song * song = entity->info.song;

		    if (entity->type!=MPD_INFO_ENTITY_TYPE_SONG)
		    {
			    mpd_freeInfoEntity(entity);
			    continue;
		    }

		    setInfo(song->title, song->artist, song->album);

		    mpd_freeInfoEntity(entity);
	    }

	    if (conn->error)
	    {
		    mpd_freeStatus(status);
		    isTicking = 0;
		    dieMPD("Connection closed");
		    return -1;
	    }
	}
    }

    mpd_finishCommand(conn);

    if (conn->error)
    {
	mpd_freeStatus(status);
	isTicking = 0;
	dieMPD("Connection closed");
	return -1;
    }

    mpd_freeStatus(status);

    if (doUpdatePlayList == 1)
        updatePlayList();

    isTicking = 0;

    return 0;
}

int MythMPD::updatePlayList()
{
    //printf("Updating playlist...\n");

    mpd_InfoEntity * entity;

    mpd_sendPlaylistInfoCommand(conn,-1);

    if (conn->error)
    {
        dieMPD("Connection closed");
	return -1;
    }

    int startWindow = songPosition - 6;
    if (startWindow < 0) startWindow = 0;
    int endWindow = startWindow + 13;

    if (ltype)
    {   
        ltype->ResetList();
        ltype->SetActive(true);

	int counter = 0;

        while ((entity = mpd_getNextInfoEntity(conn)))
        {
	    if (entity->type != MPD_INFO_ENTITY_TYPE_SONG)
	    {
	        mpd_freeInfoEntity(entity);
		continue;
	    }

	    mpd_Song * song = entity->info.song;
	    int thisPosition = song->pos;

	    if (thisPosition >= startWindow && thisPosition <= endWindow)
	    {
                char buf[256];
		sprintf(buf, "%s - %s", song->artist, song->title);

		ltype->SetItemText(counter, buf);

		if (thisPosition == songPosition && !isStopped)
		{
	            ltype->SetItemCurrent(counter);
		}

		counter++;
	    }

	    mpd_freeInfoEntity(entity);
	}

	if (isStopped)
	{
            ltype->SetItemCurrent(-1);
	}

	updateForeground();
    }

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

void MythMPD::setInfo(char *title, char *artist, char *album)
{
    getUITextType("track")->SetText(title);
    getUITextType("artist")->SetText(artist);
    getUITextType("album")->SetText(album);
}

void MythMPD::setElapsedTime(int elapsedSeconds, int totalSeconds)
{
    char buf[200];
    char es[100];
    char ts[100];

    strcpy(es, secondsToMinutesAndSeconds(elapsedSeconds));
    strcpy(ts, secondsToMinutesAndSeconds(totalSeconds));

    sprintf(buf, "%s/%s", es, ts);
    getUITextType("time")->SetText(buf);
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

void MythMPD::keyPressEvent(QKeyEvent *e)
{
    bool handled = false;

    QStringList actions;
    gContext->GetMainWindow()->TranslateKeyPress("Music", e, actions);

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
            setStatusTextTimer(buf);
	    
            mpd_sendSetvolCommand(conn, volume);
	    mpd_finishCommand(conn);
	}
	else if (action == "VOLUMEUP")
	{
	    volume += 5;
	    if (volume > 100) volume = 100;

	    char buf[16];
	    sprintf(buf, "Volume %d%%", volume);
            setStatusTextTimer(buf);

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

    if (!handled)
        MythThemedDialog::keyPressEvent(e);
}

void MythMPD::closeEvent(QCloseEvent *e)
{
    //printf("Cleaning up...\n");

    if (conn != NULL)
    {
	    mpd_closeConnection(conn);
	    conn = NULL;
    }

    if (timer != NULL)
    {
	timer->stop();
    	//timer->deleteLater();
	timer = NULL;
    }

    if (statusTimer != NULL)
    {
	statusTimer->stop();
	//statusTimer->deleteLater();
	statusTimer = NULL;
    }

    /*
    if (ltype != NULL)
    {
    	ltype->ResetList();
    	ltype->SetActive(false);
    	ltype->deleteLater();
	ltype = NULL;
    }
    */

    reset();

    e->accept();

    //printf("Done.\n");
}

MythMPD::~MythMPD() { }

#endif
