#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_DATA, "Hello World!\n");
    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

