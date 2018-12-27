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
        event_place(channel->o.header.target, &channel->o);

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

static unsigned int onkill(struct event_channel *channel)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    file_open(FILE_G0);

    while (!status)
    {

        switch (event_pick(&channel))
        {

        case EVENT_DATA:
            status = ondata(&channel);

            break;

        case EVENT_INIT:
            status = oninit(&channel);

            break;

        case EVENT_KILL:
            status = onkill(&channel);

            break;

        }

    }

    file_close(FILE_G0);

}

