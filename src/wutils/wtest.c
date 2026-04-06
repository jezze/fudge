#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int wm = lookup(option_getstring("wm-service"));

    channel_send(0, wm, EVENT_WMMAP);

    while (channel_process(0) != EVENT_WMCLOSE);

    channel_send(0, wm, EVENT_WMUNMAP);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" label \"About\"\n"
        "  + panel id \"base\" in \"window\" flow \"vertical\" padding \"1\" span \"1\"\n"
        "    + image in \"base\" mimetype \"image/pcx\" source \"initrd:data/giant.pcx\"\n";

    channel_send_fmt0(0, source, EVENT_WMRENDERDATA, data);

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_open();

    while (channel_process(0));

    channel_close();

}

