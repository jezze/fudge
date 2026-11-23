#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int count = option_getdecimal("count");
    unsigned int i;

    for (i = 0; i < count; i++)
        channel_send(0, message->source, MESSAGE_TYPE_DATA, sizeof (unsigned int), &i);

}

void init(void)
{

    option_add("count", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

