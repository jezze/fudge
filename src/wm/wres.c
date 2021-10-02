#include <fudge.h>
#include <abi.h>
#include <widget.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwmmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmmousepress *wmmousepress = mdata;

    switch (wmmousepress->button)
    {

    case 0x01:
        if (file_walk2(FILE_L0, "system:video/if:0/ctrl"))
        {

            struct ctrl_videosettings settings;

            ctrl_setvideosettings(&settings, 1920, 1080, 4);
            file_seekwriteall(FILE_L0, &settings, sizeof (struct ctrl_videosettings), 0);

        }

        break;

    }

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMMOUSEPRESS, onwmmousepress);

}

