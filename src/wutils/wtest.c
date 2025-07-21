#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    option_setdecimal("wm-service", lookup("wm-service"));
    channel_send(0, option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process(0));

    channel_send(0, option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Test\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\" padding \"1\"\n"
        "    + image in \"base\" mimetype \"image/pcx\" source \"initrd:data/giant.pcx\"\n";

    channel_send_fmt0(0, option_getdecimal("wm-service"), EVENT_WMRENDERDATA, data);

}

void init(void)
{

    option_add("wm-service", "");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

}

