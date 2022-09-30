#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwmclick(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmclick *wmclick = mdata;

    if (cstring_match(wmclick->clicked, "button0"))
    {

        char *data0 = "= text1 content \"Button 0 clicked\"\n";

        file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data0), data0);

    }

    else if (cstring_match(wmclick->clicked, "button1"))
    {

        char *data1 = "= text1 content \"Button 1 clicked\"\n";

        file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data1), data1);
    }

    else
    {

        char *data2 = "= text1 content \"No button clicked\"\n";

        file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data2), data2);

    }

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Test\"\n"
        "+ container id \"base\" in \"window\" layout \"vertical\" padding \"16\"\n"
        "+ text in \"base\" wrap \"word\" content \"Hello World! How are we today?\nI am very good thank you for asking!\nThat is very good.\"\n"
        "+ select id \"myselect\" in \"base\" label \"Select country\"\n"
        "+ button id \"button0\" in \"base\" label \"Click Me\"\n"
        "+ text id \"text1\" wrap \"word\" in \"base\" content \"<awaiting event>\"\n"
        "+ button id \"button1\" in \"base\" label \"Click Me Too\"\n"
        "+ image in \"base\" type \"image/pcx\" source \"initrd:data/mi.pcx\"\n";

    file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data), data);

}

void init(void)
{

    file_walk2(FILE_G0, "system:service/wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMCLICK, onwmclick);
    channel_bind(EVENT_WMINIT, onwminit);

}

