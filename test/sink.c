#include <fudge.h>
#include <abi.h>

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static unsigned int owner;
static unsigned int position;
static unsigned int pages;
static unsigned int wrong;

static unsigned char expected(unsigned int doubling, unsigned int at)
{

    return (doubling) ? at / 2 : ~at;

}

static void wait(unsigned int delay)
{

    volatile unsigned int n;

    for (n = 0; n < delay; n++);

}

static void ondata(struct message *message)
{

    unsigned int doubling = cstring_match(option_getstring("mode"), "double");
    struct block *block = message->data;
    unsigned int i;

    for (i = 0; i < message->length; i++)
    {

        if (block->bytes[i] != expected(doubling, position + i) && !wrong++)
        {

            unsigned int at = position + i;

            channel_send_fmt(0, owner, MESSAGE_TYPE_ERROR, "sink got a wrong byte at %u\n", &at);

        }

    }

    position += message->length;

    if (++pages == option_getdecimal("report"))
        channel_send_fmt(0, owner, MESSAGE_TYPE_DATA, "sink running\n");

    wait(option_getdecimal("delay"));

}

static void onmain(struct message *message)
{

    owner = message->source;

}

/* the stage before ends after all it gave, so everything has arrived by now */
static void onterm(struct message *message)
{

    unsigned int total = option_getdecimal("bytes") * ((cstring_match(option_getstring("mode"), "double")) ? 2 : 1);

    if (position != total)
        channel_send_fmt(0, owner, MESSAGE_TYPE_ERROR, "sink got %u bytes, expected %u\n", &position, &total);

}

void init(void)
{

    option_add("mode", "invert");
    option_add("bytes", "0");
    option_add("report", "0");
    option_add("delay", "0");
    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_TERM, onterm);

}

