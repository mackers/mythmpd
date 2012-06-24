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
#include "libmpdclient.h"
#include "mythmpd.h"

#define LOC_ERR QString("MythMPD:MAIN Error: ")
#define LOC_WARN QString("MythMPD:MAIN Warning: ")
#define LOC QString("MythMPD:MAIN: ")

using namespace std;

extern "C" {
    int mythplugin_init(const char *libversion);
    int mythplugin_run(void);
    //int mythplugin_config(void);
}
void runMythMPD(void);
int  RunMythMPD(void);
void setupKeys(void);

unsigned int *g_executed;
mpd_Connection *conn;

void setupKeys(void)
{
    REG_JUMP("MythMPD", "", "", runMythMPD);
}


int mythplugin_init(const char *libversion)
{
	LOG(VB_GENERAL, VB_GENERAL, LOC + "init");
    if (!gContext->TestPopupVersion("mythmpd", libversion,
                                    MYTH_BINARY_VERSION))
    {
        LOG(VB_GENERAL, VB_GENERAL, 
                QString("libmythmpd.so/main.o: binary version mismatch"));
        return -1;
    }

    if (!(g_executed = new unsigned int))
    	return -1;
    *g_executed = 0;

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
    
    MythMPD *mythmpd = new MythMPD(mainStack, g_executed, "MythMPD");
    
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
    LOG(VB_GENERAL, VB_GENERAL, LOC + "exec");
    return RunMythMPD();
}

int mythplugin_config(void)
{
    return 0;
}

/* plugin clean-up */
void mythplugin_destroy(void)
{
	LOG(VB_GENERAL, VB_GENERAL, LOC + "destroy");
}

