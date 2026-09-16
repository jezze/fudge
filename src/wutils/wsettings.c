#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(0, wm, EVENT_WMMAP, 0, 0);

        while (channel_process(0) != EVENT_WMCLOSE);

        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wsettings.alfi";

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

    while (channel_process(0));

}

