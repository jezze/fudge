#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, "system:service/wm"))
        channel_warning("Could not open window manager service");

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwmclick(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Filesystem Explorer\"\n"
        "+ container id \"base\" in \"window\" layout \"vertical\" padding \"8\"\n"
        "+ container id \"top\" in \"base\" layout \"horizontal\" padding \"8\"\n"
        "+ select id \"drive\" in \"top\" label \"Drives\"\n"
        "+ container id \"drivelist\" in \"drive\" layout \"vertical\" placement \"stretched\"\n"
        "+ choice id \"initrd\" in \"drivelist\" label \"initrd:\"\n"
        "+ choice id \"system\" in \"drivelist\" label \"system:\"\n"
        "+ textbox id \"path\" in \"top\" mode \"readonly\"\n"
        "+ text in \"path\" content \"/home\"\n"
        "+ container id \"main\" in \"base\" layout \"maximize\" padding \"8\"\n"
        "+ textbox id \"content\" in \"main\" mode \"readonly\"\n";

    file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data), data);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMCLICK, onwmclick);
    channel_bind(EVENT_WMINIT, onwminit);

}

