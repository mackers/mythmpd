// MythTV headers

#include <QObject>
#include <mythscreentype.h>
#include <mythuibutton.h>
#include <mythuibuttonlist.h>
#include <mythtv/uitypes.h>
#include <mythuitext.h>
#include <mythdialogbox.h>

class MythMPD_PlayQueue : public MythScreenType
{
    Q_OBJECT
    
    public:
        MythMPD_PlayQueue(MythScreenStack *parent, MythScreenType *generalScreen,
                     const char *name = 0);
        bool Create(void);
        bool keyPressEvent(QKeyEvent *event);
        void Close(void);

    private:
        MythScreenType *m_generalScreen;
        MythUIButtonList *m_buttonlistPlayQueue;
        MythUIButton   *m_buttonBack;

    private slots:
        void clicked_Back(void);
  	void clicked_track(MythUIButtonListItem *playlist_item);
};

