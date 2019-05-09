#include <fudge.h>
#include <abi.h>
#include <widget/widget.h>
#include <widget/render.h>

static unsigned int (*signals[EVENTS])(struct event_channel *channel);

static unsigned int oninit(struct event_channel *channel)
{

    event_request(channel, EVENT_WMMAP, 0);
    file_writeall(FILE_G0, &channel->o, channel->o.header.length);

    return 0;

}

static unsigned int onkill(struct event_channel *channel)
{

    event_request(channel, EVENT_WMUNMAP, 0);
    file_writeall(FILE_G0, &channel->o, channel->o.header.length);

    return 1;

}

static unsigned int onwmmousepress(struct event_channel *channel)
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

    return 0;

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(signals);
    event_setsignal(signals, EVENT_INIT, oninit);
    event_setsignal(signals, EVENT_KILL, onkill);
    event_setsignal(signals, EVENT_WMMOUSEPRESS, onwmmousepress);

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    file_open(FILE_G0);

    while (event_listen(signals, &channel));

    file_close(FILE_G0);

}

