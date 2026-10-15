#include <fudge.h>
#include <abi.h>

#define BUFFER_SIZE                     0x10000

static char buffer[BUFFER_SIZE];
static unsigned int start;
static unsigned int length;

static char getbyte(unsigned int offset)
{

    return buffer[(start + offset) % BUFFER_SIZE];

}

static void store(unsigned int source, void *data, unsigned int count)
{

    char *b = data;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        buffer[(start + length) % BUFFER_SIZE] = b[i];

        if (length < BUFFER_SIZE)
            length++;
        else
            start = (start + 1) % BUFFER_SIZE;

    }

}

static void flush(unsigned int source)
{

    unsigned int max = option_getdecimal("n");
    unsigned int lines = 0;
    unsigned int offset = length;

    if (max && offset && getbyte(offset - 1) == '\n')
        offset--;

    while (max && offset > 0)
    {

        if (getbyte(offset - 1) == '\n' && ++lines == max)
            break;

        offset--;

    }

    while (offset < length)
    {

        unsigned int index = (start + offset) % BUFFER_SIZE;
        unsigned int count = length - offset;

        if (count > BUFFER_SIZE - index)
            count = BUFFER_SIZE - index;

        if (count > MESSAGE_SIZE)
            count = MESSAGE_SIZE;

        channel_send(0, source, EVENT_DATA, count, buffer + index);

        offset += count;

    }

    start = 0;
    length = 0;

}

static void ondata(struct message *message)
{

    store(message->source, message->data, message->length);

}

static void onpath(struct message *message)
{

    if (fs_read_each(1, message->source, message->data, store))
        flush(message->source);

}

static void onterm(struct message *message)
{

    flush(message->source);

}

void init(void)
{

    option_add("n", "10");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

