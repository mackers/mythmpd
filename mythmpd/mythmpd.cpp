// MythMPD headers
#include "mythmpd.h"

#define LOC      QString("MythMPD: ")
#define LOC_WARN QString("MythMPD, Warning: ")
#define LOC_ERR  QString("MythMPD, Error: ")
extern mpd_connection *conn;

/** \brief Creates a new MythMPD Screen
*  \param parent Pointer to the screen stack
*  \param name The name of the window
*/
MythMPD::MythMPD(MythScreenStack * parent, unsigned int *executed,
		 QString name):MythScreenType(parent, name),
m_buttonPlayQueue(NULL), m_buttonArtists(NULL), m_buttonPlaylists(NULL)
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
    UIUtilE::Assign(this, m_textTitle, "text_Title", &err);
    UIUtilE::Assign(this, m_textAlbum, "text_Album", &err);
    UIUtilE::Assign(this, m_textArtist, "text_Artist", &err);
    UIUtilE::Assign(this, m_playtimeText, "text_Time", &err);
    UIUtilE::Assign(this, m_statusText, "status", &err);
    UIUtilE::Assign(this, m_MPDinfoText, "MPDinfo", &err);
    UIUtilE::Assign(this, m_buttonArtists, "button_Artists", &err);
    UIUtilE::Assign(this, m_buttonPlaylists, "button_Playlists", &err);
    UIUtilE::Assign(this, m_buttonPlayQueue, "button_PlayQueue", &err);
    UIUtilE::Assign(this, m_buttonlistPlayQueue, "buttonlist_PlayQueue",
		    &err);


    if (err) {
	    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "Cannot load screen 'MythMPD'");
	    return false;
    }

    BuildFocusList();
    buttonlist_mode = QUEUE;
    clearInformationText();

    connect(m_buttonArtists, SIGNAL(Clicked()), this,
	    SLOT(clicked_Artists()));
    connect(m_buttonPlaylists, SIGNAL(Clicked()), this,
	    SLOT(clicked_Playlists()));
    connect(m_buttonPlayQueue, SIGNAL(Clicked()), this,
	    SLOT(clicked_PlayQueue()));
    connect(m_buttonlistPlayQueue,
	    SIGNAL(itemClicked(MythUIButtonListItem *)), this,
	    SLOT(clicked_track()));


    updatePlayList();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    timer->start(1500);
    return true;
}

void MythMPD::my_finishCommand(struct mpd_connection *conn)
{
    if (!mpd_response_finish(conn))
	    printErrorAndExit(conn);
}


bool MythMPD::keyPressEvent(QKeyEvent * event)
{
    if (GetFocusWidget() && GetFocusWidget()->keyPressEvent(event))
	    return true;


    bool handled = false;
    QStringList actions;
	LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "keypress");
    handled = GetMythMainWindow()->TranslateKeyPress("Music", event, actions);

    for (int i = 0; i < actions.size() && !handled; i++) {
	    QString action = actions[i];
	    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, action);
	    handled = true;

	    if (action == "PAUSE") {
	        if (play_state == MPD_STATE_PLAY) {
		        mpd_send_pause(conn, true);
	        } else {
		        mpd_run_play(conn);
	        }
	    } else if (action == "NEXTTRACK") {
	        mpd_run_next(conn);
	    } else if (action == "PREVTRACK") {
	        mpd_run_previous(conn);
	    } else if (action == "FFWD") {
	    } else if (action == "RWND") {
	    } else if (action == "STOP") {
	        mpd_run_stop(conn);
	    } else if (action == "VOLUMEDOWN") {
	        volume -= 5;
	        if (volume < 0)
		        volume = 0;

	        char buf[16];
	        sprintf(buf, "Volume: %d%%", volume);
	        setStatus(buf);
	        mpd_run_set_volume(conn, volume);
	    } else if (action == "VOLUMEUP") {
	        volume += 5;
	        if (volume > 100)
		        volume = 100;

	        char buf[16];
	        sprintf(buf, "Volume %d%%", volume);
	        setStatus(buf);
	        mpd_run_set_volume(conn, volume);
	    } else if (action == "MUTE") {
	        if (previousVolume == 0) {
		        previousVolume = volume;
		        volume = 0;
	        } else {
		        volume = previousVolume;
		        previousVolume = 0;
	        }

	        mpd_run_set_volume(conn, volume);
	    } else if (action == "SELECT") {
	    } else {
	        handled = false;
	    }
    }

    if (!handled && MythScreenType::keyPressEvent(event))
	    handled = true;

    my_finishCommand(conn);
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

char *MythMPD::secondsToMinutesAndSeconds(int seconds)
{
    float s = seconds;
    static char buf[100];
    int h, m;

    s = (int) (10. * s + .5) / 10.;
    h = (int) (s / 3600.);
    s -= 3600 * h;
    m = (int) (s / 60.);
    s -= 60 * m;
    int s2 = (int) s;
    sprintf(buf, "%02d:%02d", m, s2);
    return buf;
}


void MythMPD::updateStatus()
{
    if (conn == NULL) {
	    if (!(connectMPD())) {
	        dieMPD((char *) "Connection closed");
	        return;
	    }
    }

    if (isTicking == 1) {
	    return;
    }

    mpd_status *status;
    int doUpdatePlayList = 0;
    isTicking = 1;

    if (!mpd_command_list_begin(conn, true) ||
	    !mpd_send_status(conn) ||
	    !mpd_send_current_song(conn) || !mpd_command_list_end(conn))
	    printErrorAndExit(conn);

    status = mpd_recv_status(conn);
    if (status == NULL)
	    printErrorAndExit(conn);

    play_state=mpd_status_get_state(status);
    if (play_state == MPD_STATE_PAUSE) {
	    setStatusText((char *) "Paused");
    } else if (play_state == MPD_STATE_PLAY) {
	    if (volume == 0) {
	        setStatusText((char *) "Muted");
	    } else {
	        setStatusText((char *) "Playing");
	    }
    } else {
	    previousSongPosition = -2;

	    setMPDinfoText((char *) "Connected");
	    setPlayInfo((char *) "No file", (char *) "Nothing Playing",
		            (char *) "");
	    m_playtimeText->SetText(QString("--/--"));
    }

    songPosition = mpd_status_get_song_pos(status);
    volume = mpd_status_get_volume(status);
    tracksInPlayList = mpd_status_get_queue_length(status);

    if (tracksInPlayList != previousTracksInPlayList) {
	    previousTracksInPlayList = tracksInPlayList;
	    doUpdatePlayList = 1;
    }

    if (mpd_status_get_state(status) == MPD_STATE_PAUSE
	    || mpd_status_get_state(status) == MPD_STATE_PLAY) {
	    if (songPosition != previousSongPosition) {
	        previousSongPosition = songPosition;
	        doUpdatePlayList = 1;
	    }

	    setElapsedTime(mpd_status_get_elapsed_time(status),
	    	       mpd_status_get_total_time(status));
	    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
	        mpd_status_free(status);
	        isTicking = 0;
	        dieMPD((char *) "Connection closed");
	        return;
	    }
        struct mpd_song *song;
        mpd_response_next(conn);
        song = mpd_recv_song(conn);

        if (song != NULL) {
            setPlayInfo((char *)song_value(song,"artist"), (char *)song_value(song,"title"), (char *)song_value(song,"album"));
            mpd_song_free(song);
        }

        if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
	        mpd_status_free(status);
            isTicking = 0;
            dieMPD((char *) "Connection closed");
            return;
        }
    }

    my_finishCommand(conn);

    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
	    mpd_status_free(status);
	    isTicking = 0;
	    dieMPD((char *) "Connection closed");
	    return;
    }

    if (doUpdatePlayList == 1) {
        doUpdatePlayList=0;
        updatePlayList();
    }

    isTicking = 0;
    return;
}


bool MythMPD::connectMPD()
{
    isConnecting = 1;
    //example of how to find the configuration dir currently used.
    QString m_sSettingsFile = GetConfDir() + "/mythmpd.conf";
    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, LOC + "Conf file:" + m_sSettingsFile);

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    QString m_server_string =
	settings.value("server", "localhost").toString();
    QString m_port_string = settings.value("port", "6600").toString();
    QString m_timeout_seconds_string =
	settings.value("timeout", "10").toString();

    QByteArray ba = m_server_string.toLocal8Bit();
    const char *m_server = ba.data();
    int m_port = m_port_string.toInt();
    float m_timeout_seconds = m_timeout_seconds_string.toFloat();

    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL,
	LOC + "Using " + m_server + ":" + m_port_string + " with " +
	m_timeout_seconds_string + " second timeout");
    conn = mpd_connection_new(m_server, m_port, m_timeout_seconds*1000);
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
	    dieMPD((char *) "Could not connect");
	    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL,
	        (char *) "MythMPD: Could not connect to mpd");
	    isConnecting = 0;
	    return false;
    }
    isConnecting = 0;

    return true;
}

void MythMPD::disconnectMPD()
{
    if (conn != NULL)
	    mpd_connection_free(conn);
    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "MythMPD: mpd connection closed");
}

void MythMPD::dieMPD(char *error)
{
    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "MPD died");
    if(timer != NULL) timer->stop();

    if (conn != NULL) {
	    fprintf(stderr, "%s\n", mpd_connection_get_error_message(conn));
        LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, mpd_connection_get_error_message(conn));
    }

    setMPDinfoText(error);
    setPlayInfo((char *) "No file", (char *) "Nothing Playing",
		(char *) "");

    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, LOC + "ERR:" + error);
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

void MythMPD::setPlayInfo(char *artist, char *title, char *album)
{
    if (not(artist)) {
	artist = (char *) "unknown";
    }
    m_textTitle->SetText(QString(title));
    m_textArtist->SetText(QString(artist));
    m_textAlbum->SetText(QString(album));
}


void MythMPD::clearInformationText()
{
    m_textTitle->SetText(QString("--"));
    m_textArtist->SetText(QString("--"));
    m_textAlbum->SetText(QString("--"));
    m_playtimeText->SetText(QString("--"));
    m_MPDinfoText->SetText(QString("--"));
}


void MythMPD::clicked_PlayQueue(void)
{
    buttonlist_mode = QUEUE;
    updatePlayList();
}

void MythMPD::clicked_Playlists(void)
{
    buttonlist_mode = PLAYLISTS;
    updatePlayList();
}

void MythMPD::clicked_Artists(void)
{
    buttonlist_mode = ARTISTS;
    updatePlayList();
}


void MythMPD::reset()
{
    conn = NULL;
    isConnecting = 0;
    play_state = 0;
    isTicking = 0;
    blockStatusTextUpdates = 0;
    volume = 80;
    previousVolume = 0;
    songPosition = -1;
    previousSongPosition = -2;
    tracksInPlayList = 1;
    previousTracksInPlayList = -1;
    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "MythMPD: Playlist cleared/reset");
}

int MythMPD::updatePlayList()
{
	struct mpd_song *song;
    LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "MythMPD: update playlist");
    if (conn == NULL) {
	    if (!(connectMPD())) {
	        dieMPD((char *) "Connection closed");
	        return -1;
	    }
    }
    setMPDinfoText((char *) "Connected");


    int startWindow = songPosition - 6;
    if (startWindow < 0)
	    startWindow = 0;

    int counter = 0;

    m_buttonlistPlayQueue->Reset();

    if (buttonlist_mode == QUEUE) {
        LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "MythMPD: Getting playlist");
        if (!mpd_send_list_queue_meta(conn)) {
            dieMPD((char *) "No songs?");
            return -1;
        }
        while ((song = mpd_recv_song(conn)) != NULL) {
	        int thisPosition = mpd_song_get_pos(song);
	        counter++;
	        char buf[256];
	        if (strlen(song_value(song,"title")) == 0) {
	            sprintf(buf, "%s / %s / %s", song_value(song,"file"), song_value(song,"artist"), song_value(song,"album"));
	        } else {
	            sprintf(buf, "%s / %s / %s", song_value(song,"title"), song_value(song,"artist"), song_value(song,"album"));
	        }
       
	        MythUIButtonListItem *playlist_item =
	            new MythUIButtonListItem(m_buttonlistPlayQueue, QString(buf));

            if (counter == songPosition) {
            }
        
	        mpd_song_free(song);
        };
    } else if (buttonlist_mode == PLAYLISTS) {
        mpd_playlist *playlist;
        LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "MythMPD: Getting playlists");
        if (!mpd_send_list_meta(conn, NULL)) {
            dieMPD((char *) "No songs?");
            return -1;
        }
        if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
            return -1;

        while ((playlist = mpd_recv_playlist(conn)) != NULL) {
            char buf[256];
            sprintf(buf, "%s", mpd_playlist_get_path(playlist));
            MythUIButtonListItem *playlist_item =
                new MythUIButtonListItem(m_buttonlistPlayQueue, QString(buf));
            mpd_playlist_free(playlist);
        };
    } else if (buttonlist_mode == ARTISTS) {
        LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL, "MythMPD: Getting artists");
        enum mpd_tag_type type;
        struct mpd_pair *pair;

        type = mpd_tag_name_iparse("artist");
        mpd_search_db_tags(conn, type);
        
        if (!mpd_search_commit(conn))
            printErrorAndExit(conn);

        QStringList artists;
        while ((pair = mpd_recv_pair_tag(conn, type)) != NULL) {
            artists.append(QString(pair->value));
            mpd_return_pair(conn, pair);
        }
        QMap<QString, QString> map;
        foreach (const QString &str, artists)
            map.insert(str.toLower(), str);
        
        artists = map.values();

        for (int i = 0; i < artists.size(); ++i) {
            MythUIButtonListItem *playlist_item =
                new MythUIButtonListItem(m_buttonlistPlayQueue, artists.at(i));
        }
    }
        
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
	    dieMPD((char *) "Connection closed");
	    return -1;
    }
    
    my_finishCommand(conn);

    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
	    dieMPD((char *) "Connection closed");
	    return -1;
    }
    
    return 0;
}

void MythMPD::clicked_track(void)
{
    int id = m_buttonlistPlayQueue->GetCurrentPos();
    if (buttonlist_mode == QUEUE) {
        LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL,
	        "MythMPD: selected track " + QString::number(id));
        mpd_run_play_pos(conn, id);
    } else if (buttonlist_mode == PLAYLISTS) {
        QByteArray ba = m_buttonlistPlayQueue->GetItemCurrent()->GetText().toLatin1();
        const char *playlist_name = ba.data();
        LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL,
	        "MythMPD: selected playlist" + ba);
        mpd_run_clear(conn);
        mpd_send_load(conn, playlist_name);
        mpd_response_finish(conn);
        mpd_run_play(conn);
        buttonlist_mode=QUEUE;
    } else if (buttonlist_mode == ARTISTS) {
        QByteArray ba = m_buttonlistPlayQueue->GetItemCurrent()->GetText().toLatin1();
        const char *playlist_name = ba.data();
        LOG((LogLevel_t)VB_GENERAL, (LogLevel_t)VB_GENERAL,
	        "MythMPD: selected artist" + ba);
        mpd_run_clear(conn);
        enum mpd_tag_type type;

        type = mpd_tag_name_iparse("artist");
        mpd_search_add_db_songs(conn, true);
        mpd_search_add_tag_constraint(conn, MPD_OPERATOR_DEFAULT, type, ba);
        
        if (!mpd_search_commit(conn))
            printErrorAndExit(conn);

        mpd_response_finish(conn);
        mpd_run_play(conn);
        buttonlist_mode=QUEUE;
    }
}

void MythMPD::printErrorAndExit(struct mpd_connection *conn)
{
    //const char *message;

    assert(mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS);

    //message = mpd_connection_get_error_message(conn);
    /*if (mpd_connection_get_error(conn) == MPD_ERROR_SERVER)
	 messages received from the server are UTF-8; the
	   rest is either US-ASCII or locale 
	message = charset_from_utf8(message);*/

	dieMPD((char *)"error: ");
}

const char *
MythMPD::song_value(const struct mpd_song *song, const char *name)
{
        static char buffer[10];
        const char *value;
        if (song ==  NULL ) {
            return "";
        }

        if (strcmp(name, "file") == 0)
                value = mpd_song_get_uri(song);
        else if (strcmp(name, "time") == 0) {
                unsigned duration = mpd_song_get_duration(song);

                if (duration > 0) {
                        snprintf(buffer, sizeof(buffer), "%d:%02d",
                                 duration / 60, duration % 60);
                        value = buffer;
                } else
                        value = NULL;
        } else if (strcmp(name, "position") == 0) {
                unsigned pos = mpd_song_get_pos(song);
                snprintf(buffer, sizeof(buffer), "%d", pos+1);
                value = buffer;
        } else if (strcmp(name, "id") == 0) {
                snprintf(buffer, sizeof(buffer), "%u", mpd_song_get_id(song));
                value = buffer;
        } else {
                enum mpd_tag_type tag_type = mpd_tag_name_iparse(name);
                if (tag_type == MPD_TAG_UNKNOWN)
                        return NULL;

                value = mpd_song_get_tag(song, tag_type, 0);
        }

        if (value == NULL)
                value = "";

        return value;
}

