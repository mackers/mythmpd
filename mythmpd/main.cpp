// C++ headers
#include <unistd.h>

// QT headers
#include <QApplication>

// MythTV headers
#include <mythcontext.h>
#include <mythplugin.h>
#include <mythpluginapi.h>
#include <mythversion.h>
#include <mythmainwindow.h>
#include <mythdbcon.h>
#include <lcddevice.h>
#include <myththemedmenu.h>
#include <mythuihelper.h>

#include "mythmpd.h"

using namespace std;

/*extern "C" {
    int mythplugin_init(const char *libversion);
    int mythplugin_run(void);
    int mythplugin_config(void);
}*/
void runMythMPD(void);
int  RunMythMPD(void);
void setupKeys(void);

void setupKeys(void)
{
    REG_JUMP("MythMPD", "", "", runMythMPD);
}


int mythplugin_init(const char *libversion)
{
    if (!gContext->TestPopupVersion("MythMPD", libversion, MYTH_BINARY_VERSION))
        return -1;
    setupKeys();
    return 0;
}
 
void runMythMPD(void)
{
    RunMythMPD();
}

int RunMythMPD(void)
{
    MythScreenStack *mainStack = GetMythMainWindow()->GetMainStack();
    
    MythMPD *mythmpd = new MythMPD(mainStack, "MythMPD");
    
    if (mythmpd->Create())
    {
        mainStack->AddScreen(mythmpd);
        return 0;
    }
    else
    {
        delete mythmpd;
        return -1;
    }
}

int mythplugin_run(void)
{
    return RunMythMPD();
}

int mythplugin_config(void)
{
    return 0;
}

