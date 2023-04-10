#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_L0, "system:service/wm"))
        channel_error("window manager service");

    file_notify(FILE_L0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(CHANNEL_DEFAULT, EVENT_WMUNMAP);
    channel_close();

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

    char *data = (char *)mdata + sizeof (struct event_wmevent);

    if (cstring_match(data, "b0"))
        channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= text1 content \"Button 0 clicked\"\n");
    else if (cstring_match(data, "b1"))
        channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= text1 content \"Button 1 clicked\"\n");
    else
        channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= text1 content \"No button clicked\"\n");

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Test\"\n"
        "  + layout id \"base\" in \"window\" form \"vertical\" padding \"8\" fit \"1\"\n"
        "    + text in \"base\" wrap \"word\" content \"Hello World! How are we today?\nI am very good thank you for asking!\nThat is very good.\"\n"
        "    + select id \"country\" in \"base\" label \"Select country\"\n"
        "      + layout id \"countrylist\" in \"country\" form \"vertical\"\n"
        "        + choice id \"sweden\" in \"countrylist\" label \"Sweden\"\n"
        "        + choice id \"norway\" in \"countrylist\" label \"Norway\"\n"
        "        + choice id \"england\" in \"countrylist\" label \"England\"\n"
        "    + button id \"b0\" in \"base\" label \"Click Me\" onclick \"b0\"\n"
        "    + text id \"text1\" wrap \"word\" in \"base\" content \"<awaiting event>\"\n"
        "    + button id \"b1\" in \"base\" label \"Click Me Too\" onclick \"b1\"\n"
        "    + image in \"base\" mimetype \"image/pcx\" source \"initrd:data/giant.pcx\"\n";

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

