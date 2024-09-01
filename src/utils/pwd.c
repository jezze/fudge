#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_fmt1(source, EVENT_DATA, "%s\n", option_getstring("pwd"));

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

