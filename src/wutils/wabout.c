#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_L0, "system:service/wm"))
        channel_panic();

    file_notify(FILE_L0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(CHANNEL_DEFAULT, EVENT_WMUNMAP);
    channel_close();

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"ABOUT\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\" padding \"8\" span \"1\"\n"
        "    + text in \"base\" weight \"bold\" wrap \"word\" content \"Fudge Operating System\"\n"
        "    + text in \"base\" wrap \"word\" content \"(more info will come soon)\"\n"
        "    + text in \"base\" weight \"bold\" wrap \"word\" content \"Contributors (in no particular order)\"\n"
        "    + text in \"base\" wrap \"word\" content \"jfu (Jens Nyberg)\"\n"
        "    + text in \"base\" wrap \"word\" content \"Max-Might\"\n"
        "    + text in \"base\" wrap \"word\" content \"tomByrer (Tom Byrer)\"\n"
        "    + text in \"base\" wrap \"word\" content \"LemonDMN2\"\n";

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

