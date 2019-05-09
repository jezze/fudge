#include <fudge.h>
#include <abi.h>

static void dump(struct event_channel *channel, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned char num[FUDGE_NSIZE];

        event_reply(channel, EVENT_DATA);
        event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0), num);
        event_place(channel->o.header.target, channel);

    }

}

static unsigned int ondata(struct event_channel *channel)
{

    dump(channel, event_getdatasize(channel), event_getdata(channel));

    return 0;

}

static unsigned int oninit(struct event_channel *channel)
{

    event_request(channel, EVENT_BLOCKREQUEST, 0);
    event_addblockrequest(&channel->o, 0, 512 * 3);
    file_writeall(FILE_G0, &channel->o, channel->o.header.length);

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

    return 0;

}

void main(void)
{

    event_initsignals();
    event_setsignal(EVENT_DATA, ondata);
    event_setsignal(EVENT_INIT, oninit);
    event_setsignal(EVENT_STOP, onstop);

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    file_open(FILE_G0);

    while (event_listen());

    file_close(FILE_G0);

}

