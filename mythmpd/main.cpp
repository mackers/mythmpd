#ifndef MAIN_CPP
#define MAIN_CPP

using namespace std;

#include "mythmpd.h"
#include <mythtv/mythcontext.h>
#include <mythtv/mythdbcon.h>
#include <mythtv/lcddevice.h>
#include <mythtv/libmythui/myththemedmenu.h>

extern "C" {
    int mythplugin_init(const char *libversion);
    int mythplugin_run(void);
    int mythplugin_config(void);
}

void runMythMPD(void);

void setupKeys(void)
{
    REG_JUMP("MythMPD", "", "", runMythMPD);

    // The rest of the keys are "borrowed" from MythMusic
}

int mythplugin_init(const char *libversion)
{
    if (!gContext->TestPopupVersion("mythmpd", libversion, MYTH_BINARY_VERSION))
        return -1;

    setupKeys();

    return 0;
}

int mythplugin_run (void)
{
    gContext->addCurrentLocation("mythmpd");

    MythMPD mpd(gContext->GetMainWindow(), "mpd", "mpd-");
    mpd.exec();

    gContext->removeCurrentLocation();

    return 1;
}

int mythplugin_config (void)
{
    return 0;
}

void runMythMPD(void)
{
    mythplugin_run();
}

#endif
