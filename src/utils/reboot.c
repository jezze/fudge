#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    /*
    unsigned int service = fs_auth(option_getstring("reset"));

    if (service)
    {

        unsigned int id = fs_walk(service, 0, option_getstring("reset"));

        if (id)
            fs_write(service, id, 0, 0, 0);
        else
            channel_send_fmt1(source, EVENT_ERROR, "Path not found: %s\n", option_getstring("reset"));

    }
    */

}

void init(void)
{

    /*
    option_add("reset", "system:reset");
    */
    channel_bind(EVENT_MAIN, onmain);

}

