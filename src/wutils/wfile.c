#include <fudge.h>
#include <abi.h>

static char path[256];

static void updatepath(void)
{

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= path content \"%s\"\n", path);

}

static void updatecontent(void)
{

    struct record records[4];
    unsigned int nrecords;

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "- content\n+ listbox id \"content\" in \"main\" mode \"readonly\"\n");
    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "+ textbutton id \"%../\" in \"content\" label \"../\"\n");
    file_walk2(FILE_PW, path);
    file_duplicate(FILE_L0, FILE_PW);

    while ((nrecords = file_list(FILE_PW, FILE_L0, 4, records)))
    {

        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            channel_sendfmt6(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "+ textbutton id \"_%w%s\" in \"content\" label \"%w%s\"\n", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "");

        }

    }

}

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

static void onwmclick(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmclick *wmclick = mdata;

    switch (wmclick->clicked[0])
    {

    case ':':
        cstring_writefmt1(path, 256, "%s:\\0", 0, wmclick->clicked + 1);
        updatepath();
        updatecontent();

        break;

    case '_':
        cstring_writefmt2(path, 256, "%s%s\\0", 0, path, wmclick->clicked + 1);
        updatepath();
        updatecontent();

        break;

    }

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Filesystem Explorer\"\n"
        "+ layout id \"base\" in \"window\" type \"vertical\"\n"
        "+ layout id \"top\" in \"base\" type \"horizontal\"\n"
        "+ layout id \"top1\" in \"top\" type \"vertical\" padding \"8\"\n"
        "+ select id \"drive\" in \"top1\" label \"Drives\"\n"
        "+ layout id \"drivelist\" in \"drive\" type \"vertical\" placement \"stretched\"\n"
        "+ choice id \":initrd\" in \"drivelist\" label \"initrd:\"\n"
        "+ choice id \":system\" in \"drivelist\" label \"system:\"\n"
        "+ layout id \"top2\" in \"top\" type \"vertical\" padding \"8\" placement \"stretched\"\n"
        "+ textbox id \"pathbox\" in \"top2\"\n"
        "+ text id \"path\" in \"pathbox\"\n"
        "+ layout id \"main\" in \"base\" type \"maximize\" padding \"8\"\n"
        "+ listbox id \"content\" in \"main\" mode \"readonly\"\n";

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data);
    cstring_writefmt0(path, 256, "initrd:\\0", 0);
    updatepath();
    updatecontent();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMCLICK, onwmclick);
    channel_bind(EVENT_WMINIT, onwminit);

}

