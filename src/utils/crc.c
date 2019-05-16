#include <fudge.h>
#include <abi.h>

static struct crc s;

static void ondata(struct event_channel *channel)
{

    crc_read(&s, event_getdata(channel), event_getdatasize(channel));

}

static void onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        crc_read(&s, buffer, count);

    file_close(file->descriptor);

}

static void onstop(struct event_channel *channel)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int result = crc_finalize(&s);

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, ascii_wvalue(buffer, 32, result, 10), buffer);
    event_append(&channel->o, 1, "\n");
    event_place(channel->o.header.target, &channel->o);

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_DATA, ondata);
    event_setsignal(&channel, EVENT_FILE, onfile);
    event_setsignal(&channel, EVENT_STOP, onstop);
    event_listen(&channel);

}

