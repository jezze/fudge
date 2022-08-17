#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        channel_redirectsame(id, EVENT_DATA);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendstringzto(id, EVENT_PATH, "/data/motd.txt");
        channel_sendto(id, EVENT_MAIN);
        channel_wait(id, EVENT_CLOSE);

    }

    else
    {

        channel_error("Could not spawn process");

    }

    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

