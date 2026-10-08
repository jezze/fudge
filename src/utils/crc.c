#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct crc sum;

static void sumdata(unsigned int source, void *buffer, unsigned int count)
{

    crc_read(&sum, buffer, count);

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

    unsigned int crc = crc_finalize(&sum);

    channel_send_fmt(0, message->source, EVENT_DATA, "%u\n", &crc);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

