#include <fudge.h>
#include <abi.h>
#include <widget.h>

static void onwmmousepress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmmousepress *wmmousepress = mdata;

    switch (wmmousepress->button)
    {

    case 0x01:
        if (file_walk2(FILE_L0, "system:video/if:0/ctrl"))
        {

            struct ctrl_videosettings settings;

            ctrl_setvideosettings(&settings, 1920, 1080, 4);
            file_open(FILE_L0);
            file_writeall(FILE_L0, &settings, sizeof (struct ctrl_videosettings));
            file_close(FILE_L0);

        }

        break;

    }

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close(channel);

}

static void onterm(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close(channel);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

    while (channel_poll(channel, &header, &data))
        channel_dispatch(channel, &header, &data);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "system:wm"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_TERM, onterm);
    channel_setcallback(channel, EVENT_WMMOUSEPRESS, onwmmousepress);

}

