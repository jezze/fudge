#include <fudge.h>
#include <abi.h>

static void oninterrupt(struct message *message)
{

    channel_send(0, channel_lookup(option_getstring("wm-service")), EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(0, wm, EVENT_WMMAP, 0, 0);
        channel_hold();

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wtest.alfi";

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_INTERRUPT, oninterrupt);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

}

