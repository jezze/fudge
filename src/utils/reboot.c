#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, "system:reset"))
        file_writeall(FILE_L0, "1", 1);
    else
        channel_error("Reset not found");

    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

