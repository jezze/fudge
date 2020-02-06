#include <fudge.h>
#include <abi.h>
#include <widget.h>

static void onwmmousepress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmmousepress *wmmousepress = mdata;

    switch (wmmousepress->button)
    {

    case 0x01:
        if (file_walk2(FILE_L0, "/system/video/if:0/ctrl"))
        {

            struct ctrl_videosettings settings;

            ctrl_setvideosettings(&settings, 1920, 1080, 4);
            file_open(FILE_L0);
            file_writeall(FILE_L0, &settings, sizeof (struct ctrl_videosettings));
            file_close(FILE_L0);

        }

        break;

    }

}

static void onwmclose(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_request(channel, EVENT_WMUNMAP);
    channel_place(channel, source);
    channel_close(channel);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_request(channel, EVENT_WMMAP);
    channel_write(channel, FILE_G0);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    file_open(FILE_G0);

}

static void onexit(struct channel *channel)
{

    file_close(FILE_G0);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_WMMOUSEPRESS, onwmmousepress);
    channel_setsignal(&channel, EVENT_WMCLOSE, onwmclose);
    channel_listen2(&channel, oninit, onexit);

}

