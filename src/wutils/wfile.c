#include <fudge.h>
#include <abi.h>

static char *path = "system:";

static void updatepath(void)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += cstring_write(buffer, BUFFER_SIZE, "= path content \"", count);
    count += cstring_write(buffer, BUFFER_SIZE, path, count);
    count += cstring_write(buffer, BUFFER_SIZE, "\"\n", count);

    file_notify(FILE_G0, EVENT_WMRENDERDATA, count, buffer);

}

static void updatecontent(void)
{

    struct record records[4];
    unsigned int nrecords;
    unsigned int cid;

    file_walk2(FILE_PW, path);

    for (cid = 0; (nrecords = file_list(FILE_PW, cid, 4, records)); cid = records[nrecords - 1].id)
    {

        char buffer[BUFFER_SIZE];
        unsigned int count = 0;
        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            count += cstring_write(buffer, BUFFER_SIZE, "+ text in \"content\" content \"", count);
            count += buffer_write(buffer, BUFFER_SIZE, record->name, record->length, count);

            if (record->type == RECORD_TYPE_DIRECTORY)
                count += cstring_write(buffer, BUFFER_SIZE, "/", count);

            count += cstring_write(buffer, BUFFER_SIZE, "\n\"\n", count);

        }

        file_notify(FILE_G0, EVENT_WMRENDERDATA, count, buffer);

    }

    channel_close();

}

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

    struct event_wmclick *wmclick = mdata;

    if (cstring_match(wmclick->clicked, "initrd"))
        path = "initrd:";
    else if (cstring_match(wmclick->clicked, "system"))
        path = "system:";

    updatepath();
    updatecontent();

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Filesystem Explorer\"\n"
        "+ container id \"base\" in \"window\" layout \"vertical\" padding \"12\"\n"
        "+ container id \"top\" in \"base\" layout \"horizontal\"\n"
        "+ container id \"top1\" in \"top\" layout \"vertical\" padding \"4\"\n"
        "+ select id \"drive\" in \"top1\" label \"Drives\"\n"
        "+ container id \"drivelist\" in \"drive\" layout \"vertical\" placement \"stretched\"\n"
        "+ choice id \"initrd\" in \"drivelist\" label \"initrd:\"\n"
        "+ choice id \"system\" in \"drivelist\" label \"system:\"\n"
        "+ container id \"top2\" in \"top\" layout \"vertical\" placement \"stretched\" padding \"4\"\n"
        "+ textbox id \"pathbox\" in \"top2\"\n"
        "+ text id \"path\" in \"pathbox\"\n"
        "+ container id \"main\" in \"base\" layout \"maximize\" padding \"4\"\n"
        "+ textbox id \"content\" in \"main\" mode \"readonly\"\n";

    file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data), data);
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

