#include <fudge.h>
#include <abi.h>
#include "kv.h"

static char path[256];
static unsigned int cursor = 0;

static void updatepath(void)
{

    channel_send_fmt1(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= path content \"%s\"\n", path);

}

static void updatecontent(void)
{

    unsigned int service = fs_auth(path);

    if (service)
    {

        unsigned int id = fs_walk(service, 0, path);

        if (id)
        {

            char data[MESSAGE_SIZE];
            struct record records[8];
            unsigned int nrecords;
            unsigned int offset;
            unsigned int maxsend = 50;
            unsigned int count = 0;

            channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "- content\n+ listbox id \"content\" in \"main\" mode \"readonly\" overflow \"vscroll\" span \"1\"\n");

            for (offset = 0; (nrecords = fs_list(service, id, offset, records, 8)); offset += nrecords)
            {

                unsigned int i;

                for (i = 0; i < nrecords; i++)
                {

                    struct record *record = &records[i];

                    count += cstring_write_fmt6(data, MESSAGE_SIZE, "+ textbutton in \"content\" label \"%w%s\" onclick \"q=relpath&path=%w%s\"\n", count, record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "");

                    if (!--maxsend)
                        return;

                }

            }

            channel_send_buffer(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, count, data);

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    lookup("wm-service");
    channel_send(option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process());

    channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmevent *event = mdata;

    if (kv_match(event, "q=copy"))
    {

    }

    else if (kv_match(event, "q=cut"))
    {

    }

    else if (kv_match(event, "q=paste"))
    {

    }

    else if (kv_match(event, "q=delete"))
    {

    }

    else if (kv_match(event, "q=up"))
    {

        if (cstring_length(path))
        {

            unsigned int l = cstring_length(path);
            unsigned int p;

            if (path[l - 1] == '/')
                l--;

            p = buffer_lastbyte(path, l, '/');

            if (!p)
                p = buffer_firstbyte(path, l, ':');

            if (p)
                path[p] = 0;

        }

        updatepath();
        updatecontent();

    }

    else if (kv_match(event, "q=abspath"))
    {

        cstring_write_fmt1(path, 256, "%s\\0", 0, kv_getstring(event, "path="));
        updatepath();
        updatecontent();

    }

    else if (kv_match(event, "q=relpath"))
    {

        cstring_write_fmt2(path, 256, "%s%s\\0", 0, path, kv_getstring(event, "path="));
        updatepath();
        updatecontent();

    }

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"File Manager\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\"\n"
        "    + layout id \"top\" in \"base\" flow \"horizontal\" padding \"1\"\n"
        "      + select id \"volume\" in \"top\" label \"Volume\"\n"
        "        + layout id \"volume-list\" in \"volume\" flow \"vertical-stretch\"\n"
        "          + choice in \"volume-list\" label \"initrd:\" onclick \"q=abspath&path=initrd:\"\n"
        "      + textbox id \"pathbox\" in \"top\" span \"1\" cursor \"0\"\n"
        "        + text id \"path\" in \"pathbox\"\n"
        "      + button in \"top\" label \"Up\" onclick \"q=up\"\n"
        "    + layout id \"main\" in \"base\" flow \"horizontal-stretch\" padding \"1\" span \"1\"\n"
        "      + listbox id \"content\" in \"main\" mode \"readonly\" overflow \"vscroll\" span \"1\"\n"
        "    + layout id \"bottom\" in \"base\" flow \"horizontal\" padding \"1\"\n"
        "      + button in \"bottom\" label \"Copy\" onclick \"q=copy\"\n"
        "      + button in \"bottom\" label \"Cut\" onclick \"q=cut\"\n"
        "      + button in \"bottom\" label \"Paste\" onclick \"q=paste\"\n"
        "      + button in \"bottom\" label \"Delete\" onclick \"q=delete\"\n";

    channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, data);
    cstring_write_fmt0(path, 256, "initrd:\\0", 0);
    updatepath();
    updatecontent();

}

static void onwmkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress *wmkeypress = mdata;

    switch (wmkeypress->id)
    {

    case KEYS_KEY_CURSORLEFT:
        if (cursor > 0)
        {

            cursor--;

            channel_send_fmt1(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= pathbox cursor \"%u\"\n", &cursor);

        }

        break;

    case KEYS_KEY_CURSORRIGHT:
        if (cursor < cstring_length(path))
        {

            cursor++;

            channel_send_fmt1(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= pathbox cursor \"%u\"\n", &cursor);

        }

        break;

    }

}

void init(void)
{

    option_add("wm-service", "");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

