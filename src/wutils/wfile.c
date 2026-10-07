#include <fudge.h>
#include <abi.h>
#include "kv.h"

static char path[256];
static unsigned int cursor = 0;

static void updatepath(unsigned int wm)
{

    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "= path label \"%s\"\n", path);

}

static void sendcontent(unsigned int wm)
{

    channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "- content\n+ listbox id \"content\" in \"main\" mode \"readonly\" flow \"vertical-stretch\" overflow \"vscroll\" span \"1\"\n");

}

static void listdirectory(unsigned int wm, unsigned int target, unsigned int id)
{

    unsigned char data[MESSAGE_SIZE];
    unsigned int count;
    unsigned int offset = 0;

    sendcontent(wm);

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

static void showfile(unsigned int wm, struct record *record)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');

    if (!start)
        start = buffer_firstbyte(path, length, ':');

    sendcontent(wm);
    channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "+ layout id \"info\" in \"content\" flow \"vertical\" padding \"8\" spacing \"4\"\n");
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" weight \"bold\" label \"%s\"\n", path + start);
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Path: %s\"\n", path);
    channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Type: File\"\n");
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Size: %u bytes\"\n", &record->size);
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Id: %u\"\n", &record->id);

}

static void updatecontent(unsigned int wm)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;
    struct record record;

    if (!id)
    {

        sendcontent(wm);
        channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "+ text in \"content\" label \"Path not found\"\n");

    }

    else if (fs_stat(1, target, id, &record) && record.type != RECORD_TYPE_DIRECTORY)
    {

        showfile(wm, &record);

    }

    else
    {

        listdirectory(wm, target, id);

    }

}

/* paths are always kept canonical: relative ones are resolved against the current path */
static void changepath(unsigned int wm, char *relative)
{

    char full[256];

    fs_absolute(full, 256, path, relative);
    cstring_write_fmt1(path, 256, 0, "%s\\0", full);
    updatepath(wm);
    updatecontent(wm);

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(0, wm, EVENT_WMMAP, 0, 0);
        channel_hold(0);
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

        char parent[256];

        cstring_write_fmt1(parent, 256, 0, "%s/../\\0", path);
        changepath(message->source, parent);

    }

    else if (kv_match(event, "q=abspath") || kv_match(event, "q=relpath"))
    {

        changepath(message->source, kv_getstring(event, "path="));

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wfile.alfi";

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    changepath(message->source, "initrd:");

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

}

