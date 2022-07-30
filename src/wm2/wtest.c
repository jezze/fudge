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
        "+ window id \"window\" title \"Test\"\n"
        "+ container id \"base\" in \"window\" layout \"vertical\" padding \"16\"\n"
        "+ text in \"base\" content \"Hello World!\nHow are we today?\"\n"
        "+ button in \"base\" label \"Click Me\"\n"
        "+ text in \"base\" content \"Hello again!\"\n"
        "+ button in \"base\" label \"Click Me Too\"\n";

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

