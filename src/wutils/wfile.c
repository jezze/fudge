#include <fudge.h>
#include <abi.h>

static char *path = "initrd:";

static void updatepath(void)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += cstring_write(buffer, BUFFER_SIZE, "= path content \"", count);
    count += cstring_write(buffer, BUFFER_SIZE, path, count);
    count += cstring_write(buffer, BUFFER_SIZE, "\"\n", count);

    channel_sendbuffer(EVENT_WMRENDERDATA, count, buffer);

}

static void updatecontent(void)
{

    struct record records[4];
    unsigned int nrecords;
    char *data =
        "- content\n"
        "+ textbox id \"content\" in \"main\" mode \"readonly\"\n";

    channel_sendfmt0(EVENT_WMRENDERDATA, data);
    file_walk2(FILE_PW, path);
    file_duplicate(FILE_L0, FILE_PW);

    while ((nrecords = file_list(FILE_PW, FILE_L0, 4, records)))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count = 0;
        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            count += cstring_write(buffer, BUFFER_SIZE, "+ textbutton in \"content\" label \"", count);
            count += buffer_write(buffer, BUFFER_SIZE, record->name, record->length, count);

            if (record->type == RECORD_TYPE_DIRECTORY)
                count += cstring_write(buffer, BUFFER_SIZE, "/", count);

            count += cstring_write(buffer, BUFFER_SIZE, "\n\"\n", count);

        }

        channel_sendbuffer(EVENT_WMRENDERDATA, count, buffer);

    }

}

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

    if (cstring_match(wmclick->clicked, "initrd"))
    {

        path = "initrd:";
        updatepath();
        updatecontent();

    }

    else if (cstring_match(wmclick->clicked, "system"))
    {

        path = "system:";
        updatepath();
        updatecontent();

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
        "+ choice id \"initrd\" in \"drivelist\" label \"initrd:\"\n"
        "+ choice id \"system\" in \"drivelist\" label \"system:\"\n"
        "+ layout id \"top2\" in \"top\" type \"vertical\" padding \"8\" placement \"stretched\"\n"
        "+ textbox id \"pathbox\" in \"top2\"\n"
        "+ text id \"path\" in \"pathbox\"\n"
        "+ layout id \"main\" in \"base\" type \"maximize\" padding \"8\"\n"
        "+ textbox id \"content\" in \"main\" mode \"readonly\"\n";

    channel_sendfmt0(EVENT_WMRENDERDATA, data);
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

