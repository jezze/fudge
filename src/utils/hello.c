#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "Hello World!\n");
    channel_replymsg(&message);
    channel_close();

}

void init(void)
{

    channel_setcallback(EVENT_MAIN, onmain);

}

