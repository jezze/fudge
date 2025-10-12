#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int wm = lookup(option_getstring("wm-service"));

    channel_send(0, wm, EVENT_WMMAP);

    while (channel_process(0));

    channel_send(0, wm, EVENT_WMUNMAP);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"About\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\" padding \"1\"\n"
        "    + text in \"base\" weight \"bold\" wrap \"word\" content \"Fudge Operating System\"\n"
        "    + text in \"base\" wrap \"word\" content \"(more info will come soon)\"\n"
        "    + text in \"base\" weight \"bold\" wrap \"word\" content \"Contributors (in no particular order)\"\n"
        "    + text in \"base\" wrap \"word\" content \"jfu (Jens Nyberg)\"\n"
        "    + text in \"base\" wrap \"word\" content \"Max-Might\"\n"
        "    + text in \"base\" wrap \"word\" content \"tomByrer (Tom Byrer)\"\n"
        "    + text in \"base\" wrap \"word\" content \"LemonDMN2\"\n";

    channel_send_fmt0(0, source, EVENT_WMRENDERDATA, data);

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

}

