#include <fudge.h>
#include <abi.h>
#include "kv.h"

static char path[256];
static unsigned int cursor = 0;

static void updatepath(unsigned int wm)
{

    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "= path label \"%s\"\n", path);

}

static void updatecontent(unsigned int wm)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, path);

        if (id)
        {

            unsigned char data[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset = 0;

            channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "- content\n+ listbox id \"content\" in \"main\" mode \"readonly\" flow \"vertical-stretch\" overflow \"vscroll\" span \"1\"\n");

            while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
            {

                unsigned char d[MESSAGE_SIZE];
                unsigned int c = 0;
                unsigned int i;

                for (i = 0; i < count; i += sizeof (struct record))
                {

                    struct record *record = (struct record *)(data + i);

                    c += cstring_write_fmt6(d, MESSAGE_SIZE, c, "+ textbutton in \"content\" label \"%w%s\" onclick \"q=relpath&path=%w%s\"\n", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "");
                    offset = record->offset;

                }

                channel_send(0, wm, EVENT_WMRENDERDATA, c, d);

            }

        }

    }

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(0, wm, EVENT_WMMAP, 0, 0);

        while (channel_process(0) != EVENT_WMCLOSE);

        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;

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

        updatepath(message->source);
        updatecontent(message->source);

    }

    else if (kv_match(event, "q=abspath"))
    {

        cstring_write_fmt1(path, 256, 0, "%s\\0", kv_getstring(event, "path="));
        updatepath(message->source);
        updatecontent(message->source);

    }

    else if (kv_match(event, "q=relpath"))
    {

        cstring_write_fmt2(path, 256, 0, "%s%s\\0", path, kv_getstring(event, "path="));
        updatepath(message->source);
        updatecontent(message->source);

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wfile.alfi";

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    cstring_write_fmt0(path, 256, 0, "initrd:\\0");
    updatepath(message->source);
    updatecontent(message->source);

}

static void onwmkeypress(struct message *message)
{

    struct event_wmkeypress *wmkeypress = message->data;

    switch (wmkeypress->id)
    {

    case KEYS_KEY_CURSORLEFT:
        if (cursor > 0)
        {

            cursor--;

            channel_send_fmt1(0, message->source, EVENT_WMRENDERDATA, "= pathbox cursor \"%u\"\n", &cursor);

        }

        break;

    case KEYS_KEY_CURSORRIGHT:
        if (cursor < cstring_length(path))
        {

            cursor++;

            channel_send_fmt1(0, message->source, EVENT_WMRENDERDATA, "= pathbox cursor \"%u\"\n", &cursor);

        }

        break;

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

    while (channel_process(0));

}

