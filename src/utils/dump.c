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
        event_append(&channel->o, 2, "  ");
        event_place(channel->o.header.target, &channel->o);

    }

}

static unsigned int ondata(struct event_channel *channel)
{

    dump(channel, event_getdatasize(channel), event_getdata(channel));

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        dump(channel, count, buffer);

    file_close(file->descriptor);

    return 0;

}

void main(void)
{

    event_initsignals();
    event_setsignal(EVENT_DATA, ondata);
    event_setsignal(EVENT_FILE, onfile);

    while (event_listen());

}

