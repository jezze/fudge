#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = call_spawn_absolute(FILE_L0, option_getstring("echo"));

    if (id)
    {

        channel_listen(id, EVENT_DATA);
        channel_listen(id, EVENT_CLOSE);
        channel_send_fmt0(id, EVENT_PATH, "/data/motd.txt\\0");
        channel_send(id, EVENT_MAIN);
        channel_wait(id, EVENT_CLOSE);
        channel_close();

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", option_getstring("echo"));

    }

}

void init(void)
{

    option_add("echo", "initrd:/bin/echo");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

