#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    channel_send_fmt(0, message->source, MESSAGE_DATA, "Hello World!\n");

}

void init(void)
{

    channel_bind(MESSAGE_MAIN, onmain);

}

