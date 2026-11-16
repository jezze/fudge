#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    channel_send(0, message->source, MESSAGE_TYPE_INTERRUPT, 0, 0);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

