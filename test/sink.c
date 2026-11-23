#include <fudge.h>
#include <abi.h>

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static unsigned char expected(unsigned int doubling, unsigned int position)
{

    return (doubling) ? position / 2 : ~position;

}

static void wait(unsigned int delay)
{

    volatile unsigned int n;

    for (n = 0; n < delay; n++);

}

static void onmain(struct message *message)
{

    unsigned int doubling = cstring_match(option_getstring("mode"), "double");
    unsigned int total = option_getdecimal("bytes") * ((doubling) ? 2 : 1);
    unsigned int report = option_getdecimal("report");
    unsigned int delay = option_getdecimal("delay");
    unsigned int position = 0;
    unsigned int pages = 0;
    unsigned int wrong = 0;

    for (;;)
    {

        struct page_data input = page_take();
        struct block *block = input.data;
        unsigned int i;

        if (!block)
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sink could not take\n");

            return;

        }

        if (!input.count)
        {

            page_free(block);

            break;

        }

        for (i = 0; i < input.count; i++)
        {

            if (block->bytes[i] != expected(doubling, position + i) && !wrong++)
            {

                unsigned int at = position + i;

                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sink got a wrong byte at %u\n", &at);

            }

        }

        position += input.count;

        if (!page_free(block))
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sink could not free\n");

        if (++pages == report)
            channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "sink running\n");

        wait(delay);

    }

    if (position != total)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sink got %u bytes, expected %u\n", &position, &total);

}

void init(void)
{

    option_add("mode", "invert");
    option_add("bytes", "0");
    option_add("report", "0");
    option_add("delay", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

