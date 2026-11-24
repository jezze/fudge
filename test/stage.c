#include <fudge.h>
#include <abi.h>

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

/* in place, so the page goes on as it came */
static void invert(unsigned int target, struct message *message)
{

    struct block *block = message->data;
    unsigned int i;

    for (i = 0; i < message->length; i++)
        block->bytes[i] = ~block->bytes[i];

    channel_send(0, target, MESSAGE_TYPE_DATA, message->length, message->data);

}

/* a filled output page is given away before the next one is taken from the pool */
static void expand(unsigned int target, struct message *message)
{

    struct block *input = message->data;
    struct block *output = 0;
    unsigned int used = 0;
    unsigned int i;

    for (i = 0; i < message->length; i++)
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

            page_give(output, target, used);

            output = 0;

        }

    }

    if (output)
        page_give(output, target, used);

}

static void ondata(struct message *message)
{

    unsigned int target = option_getdecimal("target");

    if (cstring_match(option_getstring("mode"), "double"))
        expand(target, message);
    else
        invert(target, message);

}

void init(void)
{

    option_add("target", "0");
    option_add("mode", "invert");
    channel_bind(MESSAGE_TYPE_DATA, ondata);

}

