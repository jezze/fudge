#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "%s\n", option_getstring("pwd"));

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

