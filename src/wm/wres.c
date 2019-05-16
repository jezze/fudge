#include <fudge.h>
#include <abi.h>
#include <widget/widget.h>
#include <widget/render.h>

static void onstop(struct event_channel *channel)
{

}

static void onwmmousepress(struct event_channel *channel)
{

    struct event_wmmousepress *wmmousepress = event_getdata(channel);

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

void main(void)
{

    struct event_channel channel;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_STOP, onstop);
    event_setsignal(&channel, EVENT_WMMOUSEPRESS, onwmmousepress);

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    file_open(FILE_G0);
    event_request(&channel, EVENT_WMMAP, 0);
    file_writeall(FILE_G0, &channel.o, channel.o.header.length);
    event_listen(&channel);
    event_request(&channel, EVENT_WMUNMAP, 0);
    file_writeall(FILE_G0, &channel.o, channel.o.header.length);
    file_close(FILE_G0);

}

