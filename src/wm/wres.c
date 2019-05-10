#include <fudge.h>
#include <abi.h>
#include <widget/widget.h>
#include <widget/render.h>

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

static unsigned int onstop(struct event_channel *channel)
{

    return 0;

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

void init(void)
{

    event_initsignals();
    event_setsignal(EVENT_INIT, oninit);
    event_setsignal(EVENT_KILL, onkill);
    event_setsignal(EVENT_STOP, onstop);
    event_setsignal(EVENT_WMMOUSEPRESS, onwmmousepress);

}

void main(void)
{

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    file_open(FILE_G0);

    while (event_listen());

    file_close(FILE_G0);

}

