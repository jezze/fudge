#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct md5 sum;

static void sumdata(unsigned int source, void *buffer, unsigned int count)
{

    md5_read(&sum, buffer, count);

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

    unsigned char digest[16];
    char output[32];
    unsigned int l = 32;
    unsigned int i;

    md5_write(&sum, digest);

    for (i = 0; i < 16; i++)
        cstring_write_value(output, 32, digest[i], 16, 2, i * 2);

    channel_send_fmt(0, message->source, EVENT_DATA, "%w\n", output, &l);

}

void init(void)
{

    md5_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

