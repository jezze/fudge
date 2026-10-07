#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define NUM_ACTIONS                     2

/* what can be done with a file, chosen by its suffix: a button with the label runs the command with the file as its path */
struct action
{

    char *suffix;
    char *label;
    char *command;

};

static struct action actions[NUM_ACTIONS] = {
    {".gb", "Play", "gameboy"},
    {".pcx", "Show", "wimage"}
};

static char path[256];

static void updatepath(unsigned int wm)
{

    unsigned int length = cstring_length(path);

    channel_send_fmt2(0, wm, EVENT_WMRENDERDATA, "= path label \"%s\"\n= pathbox cursor \"%u\"\n", path, &length);

}

static void sendcontent(unsigned int wm)
{

    channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "- content\n+ listbox id \"content\" in \"main\" mode \"readonly\" flow \"vertical-stretch\" overflow \"vscroll\" span \"1\"\n");

}

static void listdirectory(unsigned int wm, unsigned int target, unsigned int id)
{

    unsigned char data[MESSAGE_SIZE];
    unsigned char d[MESSAGE_SIZE];
    unsigned int count;
    unsigned int offset = 0;
    unsigned int row = 0;
    unsigned int c = 0;

    sendcontent(wm);

    while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
    {

        unsigned int i;

        for (i = 0; i < count; i += sizeof (struct record))
        {

            struct record *record = (struct record *)(data + i);
            char name[RECORD_NAMESIZE + 2];

            cstring_write_fmt3(name, RECORD_NAMESIZE + 2, 0, "%w%s\\0", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "");

            if (c + 512 > MESSAGE_SIZE)
            {

                channel_send(0, wm, EVENT_WMRENDERDATA, c, d);

                c = 0;

            }

            c += cstring_write_fmt4(d, MESSAGE_SIZE, c, "+ item id \"row%u\" in \"content\" padding \"4\" spacing \"8\" onclick \"q=relpath&path=%s\"\n+ checkbox in \"row%u\"\n+ text in \"row%u\" valign \"middle\"", &row, name, &row, &row);
            c += cstring_write_fmt1(d, MESSAGE_SIZE, c, " label \"%s\"\n", name);
            offset = record->offset;
            row++;

        }

    }

    if (c)
        channel_send(0, wm, EVENT_WMRENDERDATA, c, d);

}

static unsigned int hassuffix(char *name, char *suffix)
{

    unsigned int length = cstring_length(name);
    unsigned int slength = cstring_length(suffix);

    return length > slength && buffer_match(name + length - slength, suffix, slength);

}

static void showfile(unsigned int wm, struct record *record)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');
    unsigned int i;

    if (!start)
        start = buffer_firstbyte(path, length, ':');

    sendcontent(wm);
    channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "+ layout id \"info\" in \"content\" flow \"vertical\" padding \"8\" spacing \"4\"\n");
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" weight \"bold\" label \"%s\"\n", path + start);
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Path: %s\"\n", path);
    channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Type: File\"\n");
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Size: %u bytes\"\n", &record->size);
    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Id: %u\"\n", &record->id);

    for (i = 0; i < NUM_ACTIONS; i++)
    {

        if (hassuffix(path, actions[i].suffix))
            channel_send_fmt2(0, wm, EVENT_WMRENDERDATA, "+ button in \"info\" label \"%s\" onclick \"q=action&index=%u\"\n", actions[i].label, &i);

    }

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

        unsigned int length = cstring_length(path);

        /* entries are joined onto the path, so a directory always ends with / (or : for a root) */
        if (length && path[length - 1] != '/' && path[length - 1] != ':')
            cstring_write_fmt0(path, 256, length, "/\\0");

        listdirectory(wm, target, id);

    }

}

/* paths are always kept canonical: relative ones are resolved against the current path */
static void changepath(unsigned int wm, char *relative)
{

    char full[256];

    fs_absolute(full, 256, path, relative);
    cstring_write_fmt1(path, 256, 0, "%s\\0", full);
    updatecontent(wm);
    updatepath(wm);

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

    else if (kv_match(event, "q=action"))
    {

        unsigned int index = kv_getvalue(event, "index=", 10);

        if (index < NUM_ACTIONS)
        {

            char command[512];

            cstring_write_fmt2(command, 512, 0, "%s \"%s\" &\\0", actions[index].command, path);
            system_run(0, command);

        }

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

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

