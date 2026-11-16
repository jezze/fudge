#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    channel_setstatus(MESSAGE_STATUS_EXIT);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

