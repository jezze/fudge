#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process());

    channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Settings\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\" padding \"1\"\n"
        "    + layout id \"languages-list\" in \"base\" flow \"horizontal\"\n"
        "      + text in \"languages-list\" wrap \"word\" span \"1\" content \"Language\"\n"
        "      + select id \"languages\" in \"languages-list\" label \"English (en_US)\"\n"
        "        + layout id \"languages-box\" in \"languages\" flow \"vertical-stretch\"\n"
        "          + choice in \"languages-box\" label \"English (en_US)\" onclick \"set language en_US\"\n"
        "          + choice in \"languages-box\" label \"English (en_GB)\" onclick \"set language en_GB\"\n"
        "          + choice in \"languages-box\" label \"Swedish (sv_SE)\" onclick \"set language sv_SE\"\n"
        "    + layout id \"keyboardlayouts-list\" in \"base\" flow \"horizontal\"\n"
        "      + text in \"keyboardlayouts-list\" wrap \"word\" span \"1\" content \"Keyboard layout\"\n"
        "      + select id \"keyboardlayouts\" in \"keyboardlayouts-list\" label \"USA (qwerty_US)\"\n"
        "        + layout id \"keyboardlayouts-box\" in \"keyboardlayouts\" flow \"vertical-stretch\"\n"
        "          + choice in \"keyboardlayouts-box\" label \"USA (qwerty_US)\" onclick \"set keyboard qwerty_US\"\n"
        "          + choice in \"keyboardlayouts-box\" label \"Sweden (qwerty_SE)\" onclick \"set keyboard qwerty_SE\"\n"
        "          + choice in \"keyboardlayouts-box\" label \"Dvorak (dvorak)\" onclick \"set keyboard dvorak\"\n"
        "    + layout id \"mouse-acc-list\" in \"base\" flow \"horizontal\"\n"
        "      + text in \"mouse-acc-list\" wrap \"word\" span \"1\" content \"Mouse acceleration\"\n"
        "      + text in \"mouse-acc-list\" weight \"bold\" wrap \"word\" content \"Yes / No\"\n";

    channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, data);

}

void init(void)
{

    option_add("wm-service", "12345");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

}

