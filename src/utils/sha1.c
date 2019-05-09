#include <fudge.h>
#include <abi.h>

static unsigned int (*signals[EVENTS])(struct event_channel *channel);
static struct sha1 s;

static unsigned int ondata(struct event_channel *channel)
{

    sha1_read(&s, event_getdata(channel), event_getdatasize(channel));

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        sha1_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

    unsigned char digest[20];
    char num[FUDGE_NSIZE];
    unsigned int i;

    sha1_write(&s, digest);
    event_reply(channel, EVENT_DATA);

    for (i = 0; i < 20; i++)
        event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0), num);

    event_append(&channel->o, 1, "\n");
    event_place(channel->o.header.target, channel);

    return 1;

}

static unsigned int oninit(struct event_channel *channel)
{

    sha1_init(&s);

    return 0;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    event_initsignals(signals);

    signals[EVENT_DATA] = ondata;
    signals[EVENT_FILE] = onfile;
    signals[EVENT_STOP] = onstop;
    signals[EVENT_INIT] = oninit;

    while (!status)
    {

        unsigned int type = event_pick(&channel);

        status = signals[type](&channel);

    }

}

