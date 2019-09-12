#include <fudge.h>
#include <abi.h>

static unsigned int bytes;
static unsigned int words;
static unsigned int lines;
static unsigned int whitespace = 1;

static void sum(unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        switch (data[i])
        {

        case '\n':
            whitespace = 1;
            lines++;

            break;

        case ' ':
            whitespace = 1;

            break;

        default:
            if (whitespace)
                words++;

            whitespace = 0;

            break;

        }

        bytes++;

    }

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    sum(msize, mdata);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        sum(count, buffer);

    file_close(file->descriptor);

}

void main(void)
{

    struct channel channel;
    char num[FUDGE_NSIZE];
    unsigned int id;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

    id = channel_reply(&channel, EVENT_DATA);

    channel_append(&channel, ascii_wvalue(num, FUDGE_BSIZE, lines, 10), num);
    channel_append(&channel, 1, "\n");
    channel_append(&channel, ascii_wvalue(num, FUDGE_BSIZE, words, 10), num);
    channel_append(&channel, 1, "\n");
    channel_append(&channel, ascii_wvalue(num, FUDGE_BSIZE, bytes, 10), num);
    channel_append(&channel, 1, "\n");
    channel_place(&channel, id);

}

