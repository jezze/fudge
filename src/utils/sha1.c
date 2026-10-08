#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct sha1 sum;

static void sumdata(unsigned int source, void *buffer, unsigned int count)
{

    sha1_read(&sum, buffer, count);

}

static void ondata(struct message *message)
{

    sumdata(message->source, message->data, message->length);

}

static void onpath(struct message *message)
{

    fs_read_each(1, message->source, message->data, sumdata);

}

static void onterm(struct message *message)
{

    unsigned char digest[20];
    char output[40];
    unsigned int l = 40;
    unsigned int i;

    sha1_write(&sum, digest);

    for (i = 0; i < 20; i++)
        cstring_write_value(output, 40, digest[i], 16, 2, i * 2);

    channel_send_fmt(0, message->source, EVENT_DATA, "%w\n", output, &l);

}

void init(void)
{

    sha1_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

