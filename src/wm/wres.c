#include <fudge.h>
#include <abi.h>
#include <widget.h>

static void onwmmousepress(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_wmmousepress *wmmousepress = mdata;

    switch (wmmousepress->button)
    {

    case 0x01:
        if (!file_walk2(FILE_L0, "/system/video/if:0/ctrl"))
            break;

        render_init();
        render_setvideo(FILE_L0, 1920, 1080, 4);

        break;

    }

}

static void onwmclose(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_request(channel, EVENT_WMUNMAP);
    channel_exit(channel);
    file_writeall(FILE_G0, &channel->o, channel->o.length);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_nosignal(&channel, EVENT_CLOSE);
    channel_setsignal(&channel, EVENT_WMMOUSEPRESS, onwmmousepress);
    channel_setsignal(&channel, EVENT_WMCLOSE, onwmclose);

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    file_open(FILE_G0);
    channel_request(&channel, EVENT_WMMAP);
    file_writeall(FILE_G0, &channel.o, channel.o.length);
    channel_listen(&channel);
    file_close(FILE_G0);

}

