#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wabout.alfi";

        channel_send(0, wm, MESSAGE_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        channel_hold(0);
        channel_send(0, wm, MESSAGE_WMUNMAP, 0, 0);

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(MESSAGE_MAIN, onmain);

}

