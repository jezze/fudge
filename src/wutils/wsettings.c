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
        "+ window id \"window\" title \"SETTINGS\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\" padding \"8\" span \"1\"\n"
        "    + layout id \"languages-list\" in \"base\" flow \"horizontal\"\n"
        "      + text in \"languages-list\" wrap \"word\" span \"1\" content \"Language\"\n"
        "      + select id \"languages\" in \"languages-list\" label \"English (en_US)\"\n"
        "        + layout id \"languages-box\" in \"languages\" flow \"vertical\"\n"
        "          + choice in \"languages-box\" label \"English (en_US)\" onclick \"set language en_US\"\n"
        "          + choice in \"languages-box\" label \"Swedish (sv_SE)\" onclick \"set language sv_SE\"\n"
        "    + layout id \"keyboardlayouts-list\" in \"base\" flow \"horizontal\"\n"
        "      + text in \"keyboardlayouts-list\" wrap \"word\" span \"1\" content \"Keyboard layout\"\n"
        "      + select id \"keyboardlayouts\" in \"keyboardlayouts-list\" label \"USA (us)\"\n"
        "        + layout id \"keyboardlayouts-box\" in \"keyboardlayouts\" flow \"vertical\"\n"
        "          + choice in \"keyboardlayouts-box\" label \"USA (us)\" onclick \"set keyboard us\"\n"
        "          + choice in \"keyboardlayouts-box\" label \"Sweden (se)\" onclick \"set keyboard se\"\n"
        "    + layout id \"mouse-acc-list\" in \"base\" flow \"horizontal\"\n"
        "      + text in \"mouse-acc-list\" wrap \"word\" span \"1\" content \"Mouse acceleration\"\n"
        "      + text in \"mouse-acc-list\" weight \"bold\" wrap \"word\" content \"Yes / No\"\n";

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

