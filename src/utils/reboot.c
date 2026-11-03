#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    channel_send(0, option_getdecimal("reset-service"), MESSAGE_TYPE_TERM, 0, 0);

}

void init(void)
{

    option_add("reset-service", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

