#include <fudge.h>
#include <abi.h>

static struct crc s;

static unsigned int ondata(struct event_channel *channel)
{

    crc_read(&s, event_getdata(channel), event_getdatasize(channel));

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        crc_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int result = crc_finalize(&s);

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, ascii_wvalue(buffer, 32, result, 10), buffer);
    event_append(&channel->o, 1, "\n");
    event_place(channel->o.header.target, &channel->o);

    return 1;

}

void init(void)
{

    event_initsignals();
    event_setsignal(EVENT_DATA, ondata);
    event_setsignal(EVENT_FILE, onfile);
    event_setsignal(EVENT_STOP, onstop);

}

void main(void)
{

    while (event_listen());

}

