#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, "system:service/wm"))
        file_notify(FILE_L0, EVENT_WMMAP, 0, 0);
    else
        channel_warning("Could not open window manager service");

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(EVENT_WMUNMAP);
    channel_close();

}

static void onwmclick(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmclick *wmclick = mdata;

    if (cstring_match(wmclick->clicked, "button0"))
        channel_sendfmt0(EVENT_WMRENDERDATA, "= text1 content \"Button 0 clicked\"\n");
    else if (cstring_match(wmclick->clicked, "button1"))
        channel_sendfmt0(EVENT_WMRENDERDATA, "= text1 content \"Button 1 clicked\"\n");
    else
        channel_sendfmt0(EVENT_WMRENDERDATA, "= text1 content \"No button clicked\"\n");

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Test\"\n"
        "+ layout id \"base\" in \"window\" type \"vertical\" padding \"8\"\n"
        "+ text in \"base\" wrap \"word\" content \"Hello World! How are we today?\nI am very good thank you for asking!\nThat is very good.\"\n"
        "+ select id \"country\" in \"base\" label \"Select country\"\n"
        "+ layout id \"countrylist\" in \"country\" type \"vertical\" placement \"stretched\"\n"
        "+ choice id \"sweden\" in \"countrylist\" label \"Sweden\"\n"
        "+ choice id \"norway\" in \"countrylist\" label \"Norway\"\n"
        "+ choice id \"england\" in \"countrylist\" label \"England\"\n"
        "+ button id \"button0\" in \"base\" label \"Click Me\"\n"
        "+ text id \"text1\" wrap \"word\" in \"base\" content \"<awaiting event>\"\n"
        "+ button id \"button1\" in \"base\" label \"Click Me Too\"\n"
        "+ image in \"base\" type \"image/pcx\" source \"initrd:data/mi.pcx\"\n";

    channel_sendfmt0(EVENT_WMRENDERDATA, data);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMCLICK, onwmclick);
    channel_bind(EVENT_WMINIT, onwminit);

}

