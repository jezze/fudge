#include <fudge.h>
#include <abi.h>

static struct md5 s;

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    md5_read(&s, mdata, msize);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        md5_read(&s, buffer, count);

    file_close(file->descriptor);

}

void main(void)
{

    struct channel channel;
    unsigned char digest[16];
    char num[FUDGE_NSIZE];
    unsigned int id;
    unsigned int i;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    md5_init(&s);
    channel_listen(&channel);
    md5_write(&s, digest);

    id = channel_reply(&channel, EVENT_DATA);

    for (i = 0; i < 16; i++)
        channel_append(&channel, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0), num);

    channel_append(&channel, 1, "\n");
    channel_place(&channel, id);

}

