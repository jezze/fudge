#include <fudge.h>
#include <abi.h>

static struct sha1 s;

static void ondata(struct event_channel *channel)
{

    sha1_read(&s, event_getdata(channel), event_getdatasize(channel));

}

static void onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        sha1_read(&s, buffer, count);

    file_close(file->descriptor);

}

void main(void)
{

    struct event_channel channel;
    unsigned char digest[20];
    char num[FUDGE_NSIZE];
    unsigned int i;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_DATA, ondata);
    event_setsignal(&channel, EVENT_FILE, onfile);
    sha1_init(&s);
    event_listen(&channel);
    sha1_write(&s, digest);
    event_reply(&channel, EVENT_DATA);

    for (i = 0; i < 20; i++)
        event_append(&channel.o, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0), num);

    event_append(&channel.o, 1, "\n");
    event_place(channel.o.header.target, &channel.o);

}

