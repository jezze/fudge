#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwmshow(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Calculator\"\n"
        "+ container id \"base\" in \"window\" layout \"vertical\"\n"
        "+ container id \"container\" in \"base\" layout \"vertical\" placement \"stretched\" padding \"16\"\n"
        "+ textbox in \"container\" content \"1337\" mode \"readonly\"\n"
        "+ grid id \"buttons\" in \"base\" columns \"4\" placement \"stretched\" padding \"16\"\n"
        "+ button in \"buttons\" label \"7\"\n"
        "+ button in \"buttons\" label \"8\"\n"
        "+ button in \"buttons\" label \"9\"\n"
        "+ button in \"buttons\" label \"/\"\n"
        "+ button in \"buttons\" label \"4\"\n"
        "+ button in \"buttons\" label \"5\"\n"
        "+ button in \"buttons\" label \"6\"\n"
        "+ button in \"buttons\" label \"x\"\n"
        "+ button in \"buttons\" label \"3\"\n"
        "+ button in \"buttons\" label \"2\"\n"
        "+ button in \"buttons\" label \"1\"\n"
        "+ button in \"buttons\" label \"-\"\n"
        "+ button in \"buttons\" label \"0\"\n"
        "+ button in \"buttons\" label \".\"\n"
        "+ button in \"buttons\" label \"+\"\n"
        "+ button in \"buttons\" label \"=\"\n";

    file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data), data);

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMSHOW, onwmshow);

}

