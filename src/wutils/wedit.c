#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define CHUNKSIZE                       768
#define MAXSIZE                         0x4000
#define SAVESIZE                        0x8000
#define NUM_WRAPS                       3

static char path[256];
static unsigned int wm;
static char *labels[NUM_WRAPS] = {"None", "Word", "Character"};
static char *wraps[NUM_WRAPS] = {"none", "word", "char"};
static unsigned int wrap = 0;
static unsigned int chunks;
static unsigned int loaded;
static char savedata[SAVESIZE];

static void sendchunk(char *data, unsigned int count)
{

    char buffer[MESSAGE_SIZE];
    unsigned int offset = cstring_write_fmt(buffer, MESSAGE_SIZE, 0, "+ text id \"chunk%u\" in \"content\" display \"inline\" wrap \"%s\" label \"", &chunks, wraps[wrap]);
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (data[i] == '"')
            offset += buffer_write(buffer, MESSAGE_SIZE, "\"\\\"\"", 4, offset);
        else
            offset += buffer_write(buffer, MESSAGE_SIZE, &data[i], 1, offset);

    }

    offset += buffer_write(buffer, MESSAGE_SIZE, "\"\n", 2, offset);

    channel_send(0, wm, EVENT_WMRENDERDATA, offset, buffer);

    chunks++;

}

static void setwrap(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int offset = cstring_write_fmt(buffer, MESSAGE_SIZE, 0, "= wrap label \"%s\"\n", labels[wrap]);
    unsigned int i;

    for (i = 0; i < chunks; i++)
        offset += cstring_write_fmt(buffer, MESSAGE_SIZE, offset, "= chunk%u wrap \"%s\"\n", &i, wraps[wrap]);

    channel_send(0, wm, EVENT_WMRENDERDATA, offset, buffer);

}

static void clearfile(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; i < chunks; i++)
        offset += cstring_write_fmt(buffer, MESSAGE_SIZE, offset, "- chunk%u\n", &i);

    if (offset)
        channel_send(0, wm, EVENT_WMRENDERDATA, offset, buffer);

    chunks = 0;
    loaded = 0;

}

static void showfile(void)
{

    unsigned int target;
    unsigned int id;

    if (!wm || !path[0])
        return;

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= window label \"Text Editor (%s)\"\n", path + fs_dirlength(path));
    target = fs_auth(path);
    id = (target) ? fs_walk(1, target, 0, path) : 0;

    if (id)
    {

        char data[CHUNKSIZE];
        unsigned int offset = 0;
        unsigned int count;

        while (offset < MAXSIZE && (count = fs_read(1, target, id, data, CHUNKSIZE, offset)))
        {

            sendchunk(data, count);

            offset += count;

        }

        if (offset >= MAXSIZE)
        {

            channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text id \"chunk%u\" in \"content\" weight \"bold\" label \"(only the first 16 KB are shown)\"\n", &chunks);

            chunks++;

        }

        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text id \"chunk%u\" in \"content\" display \"inline\" wrap \"%s\"\n= content cursor \"0\"\n", &chunks, wraps[wrap]);

        chunks++;
        loaded = offset < MAXSIZE;

    }

    else
    {

        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text id \"chunk%u\" in \"content\" label \"Could not open %s\"\n", &chunks, path);

        chunks++;

    }

}

static void writefile(void)
{

    char *name = path + fs_dirlength(path);
    unsigned int count = 0;
    unsigned int total = 0;
    struct message message;
    unsigned int target;
    unsigned int parent;
    unsigned int id;

    channel_send(0, wm, EVENT_WMREAD, 8, "content");

    while (channel_poll(0, wm, EVENT_WMTEXT, &message) && message.length)
    {

        count += buffer_write(savedata, SAVESIZE, message.data, message.length, count);
        total += message.length;

    }

    if (total != count)
        return;

    target = fs_auth(path);
    parent = (target) ? fs_walkparent(1, target, path) : 0;
    id = (parent) ? fs_walk(1, target, 0, path) : 0;

    if (id && fs_remove(1, target, parent, id))
    {

        id = fs_create(1, target, parent, name, cstring_length(name));

        if (id)
            fs_write_all(1, target, id, savedata, count, 0);

    }

}

static void save(void)
{

    static unsigned int saving;

    if (!loaded || saving)
        return;

    saving = 1;
    writefile();
    saving = 0;

}

static void choose(void)
{

    unsigned int target = fs_spawn(1, "initrd:bin/wfile");

    if (target)
    {

        channel_send_fmt(0, target, EVENT_OPTION, "mode\\0select\\0wm-service\\0%s\\0", option_getstring("wm-service"));
        channel_send(0, target, EVENT_MAIN, 0, 0);
        channel_send(0, target, EVENT_TERM, 0, 0);

    }

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wedit.alfi";

        channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        showfile();
        channel_hold(0);
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onpath(struct message *message)
{

    clearfile();
    cstring_write_fmt(path, 256, 0, "%s\\0", message->data);
    showfile();

}

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;

    if (kv_match(event, "q=wrap"))
    {

        char *label = kv_getstring(event, "wrap=");
        unsigned int i;

        for (i = 0; i < NUM_WRAPS; i++)
        {

            if (label && cstring_match(label, labels[i]))
                wrap = i;

        }

        setwrap();

    }

    if (kv_match(event, "q=open"))
        choose();

    if (kv_match(event, "q=save"))
        save();

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_WMEVENT, onwmevent);

}

