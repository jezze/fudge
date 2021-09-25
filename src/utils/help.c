#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        channel_sendredirectsame(id, EVENT_DATA);
        channel_sendredirectback(id, EVENT_CLOSE);
        channel_sendstringz(id, EVENT_PATH, "/data/help.txt");
        channel_send(id, EVENT_MAIN);
        channel_wait(id, EVENT_CLOSE);

    }

    channel_close();

}

void init(void)
{

    channel_setcallback(EVENT_MAIN, onmain);

}

