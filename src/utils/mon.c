#include <fudge.h>
#include <abi.h>

static void dump(struct channel *channel, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned char num[FUDGE_NSIZE];

        channel_reply(channel, EVENT_DATA);
        event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0), num);
        channel_place(channel->o.header.target, &channel->o);

    }

}

static void onstop(struct channel *channel)
{

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_STOP, onstop);
    channel_clearsignal(&channel, EVENT_DATA);

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    file_open(FILE_G0);
    channel_request(&channel, EVENT_BLOCKREQUEST, 0);
    event_addblockrequest(&channel.o, 0, 512 * 3);
    file_writeall(FILE_G0, &channel.o, channel.o.header.length);
    channel_listen(&channel);
    dump(&channel, channel_getdatasize(&channel), channel_getdata(&channel));
    file_close(FILE_G0);

}

