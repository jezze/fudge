#include <fudge.h>
#include <abi.h>

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static void onmain(struct message *message)
{

    unsigned int target = option_getdecimal("target");
    unsigned int total = option_getdecimal("bytes");
    unsigned int sent = 0;
    unsigned int announced = 0;

    while (sent < total)
    {

        struct block *block = page_tryalloc();
        unsigned int count = (total - sent < PAGE_SIZE) ? total - sent : PAGE_SIZE;
        unsigned int i;

        /* the owner learns once that the pool ran dry, before this task sleeps for the first time */
        if (!block)
        {

            if (!announced++)
                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "source waiting\n");

            block = page_alloc();

        }

        for (i = 0; i < count; i++)
            block->bytes[i] = sent + i;

        /* so a test can see the page arrive at the end of a pipe without having been copied */
        if (!sent && option_getdecimal("report"))
        {

            unsigned int address = (unsigned long)block;

            channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "%u", &address);

        }

        if (!page_give(block, target, count))
        {

            page_free(block);
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "source could not give at byte %u\n", &sent);

            return;

        }

        sent += count;

    }

    if (!page_give(page_alloc(), target, 0))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "source could not end the stream\n");

}

void init(void)
{

    option_add("target", "0");
    option_add("bytes", "0");
    option_add("report", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

