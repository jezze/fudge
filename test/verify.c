#include <fudge.h>
#include <abi.h>

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static unsigned int owner;
static unsigned int position;
static unsigned int wrong;
static unsigned int first;

static void wait(unsigned int delay)
{

    volatile unsigned int n;

    for (n = 0; n < delay; n++);

}

/* what emit writes after tr -f a -t b */
static unsigned char expected(unsigned int at)
{

    unsigned char c = 'a' + at % 26;

    return (c == 'a') ? 'b' : c;

}

static void ondata(struct message *message)
{

    struct block *block = message->data;
    unsigned int i;

    for (i = 0; i < message->length; i++)
    {

        if (block->bytes[i] != expected(position + i) && !wrong++)
            first = position + i;

    }

    position += message->length;

    wait(option_getdecimal("delay"));

}

static void onmain(struct message *message)
{

    owner = message->source;

}

/* the pages given before the end are all read by now; the verdict goes on down the pipe */
static void onterm(struct message *message)
{

    unsigned int total = option_getdecimal("bytes");

    if (wrong)
        channel_send_fmt(0, owner, MESSAGE_TYPE_DATA, "verify fail: %u bytes differ, first at %u\n", &wrong, &first);
    else if (position != total)
        channel_send_fmt(0, owner, MESSAGE_TYPE_DATA, "verify fail: %u of %u bytes\n", &position, &total);
    else
        channel_send_fmt(0, owner, MESSAGE_TYPE_DATA, "verify ok %u\n", &position);

}

void init(void)
{

    option_add("bytes", "0");
    option_add("delay", "0");
    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_TERM, onterm);

}

