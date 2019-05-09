#include <fudge.h>
#include <abi.h>

static unsigned int (*signals[EVENTS])(struct event_channel *channel);
static struct md5 s;

static unsigned int ondata(struct event_channel *channel)
{

    md5_read(&s, event_getdata(channel), event_getdatasize(channel));

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        md5_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

    unsigned char digest[16];
    char num[FUDGE_NSIZE];
    unsigned int i;

    md5_write(&s, digest);
    event_reply(channel, EVENT_DATA);

    for (i = 0; i < 16; i++)
        event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0), num);

    event_append(&channel->o, 1, "\n");
    event_place(channel->o.header.target, channel);

    return 1;

}

static unsigned int oninit(struct event_channel *channel)
{

    md5_init(&s);

    return 0;

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(signals);
    event_setsignal(signals, EVENT_DATA, ondata);
    event_setsignal(signals, EVENT_FILE, onfile);
    event_setsignal(signals, EVENT_STOP, onstop);
    event_setsignal(signals, EVENT_INIT, oninit);

    while (event_listen(signals, &channel));

}

