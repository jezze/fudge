#include <fudge.h>
#include <abi.h>

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static unsigned int invert(unsigned int target, struct block *input, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        input->bytes[i] = ~input->bytes[i];

    return page_give(input, target, count);

}

/* a filled output page is given away before the next one is taken from the pool */
static unsigned int expand(unsigned int target, struct block *input, unsigned int count)
{

    struct block *output = 0;
    unsigned int used = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!output)
        {

            output = page_alloc();
            used = 0;

        }

        output->bytes[used++] = input->bytes[i];
        output->bytes[used++] = input->bytes[i];

        if (used == PAGE_SIZE)
        {

            if (!page_give(output, target, used))
                return 0;

            output = 0;

        }

    }

    if (output && !page_give(output, target, used))
        return 0;

    return page_free(input);

}

static void onmain(struct message *message)
{

    unsigned int target = option_getdecimal("target");
    unsigned int doubling = cstring_match(option_getstring("mode"), "double");

    for (;;)
    {

        struct page_data input = page_take(0);

        if (!input.data)
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "stage could not take\n");

            return;

        }

        if (!input.count)
        {

            if (!page_give(input.data, target, 0))
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "stage could not end the stream\n");

            return;

        }

        if (!((doubling) ? expand(target, input.data, input.count) : invert(target, input.data, input.count)))
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "stage could not give\n");

            return;

        }

    }

}

void init(void)
{

    option_add("target", "0");
    option_add("mode", "invert");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

