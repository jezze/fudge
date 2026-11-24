#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define CHUNKSIZE                       768
#define MAXSIZE                         0x4000
#define SAVESIZE                        0x8000
#define NUM_WRAPS                       3
#define CHOOSE_OPEN                     1
#define CHOOSE_SAVE                     2

static char path[256];
static unsigned int wm;
static char *labels[NUM_WRAPS] = {"None", "Word", "Character"};
static char *wraps[NUM_WRAPS] = {"none", "word", "char"};
static unsigned int wrap = 0;
static unsigned int chunks;
static unsigned int loaded;
static unsigned int exists;
static unsigned int saving;
static unsigned int choosing;
static char savedata[SAVESIZE];

static void sendcursor(unsigned int position)
{

    struct {struct message_wmcursor wmcursor; char id[8];} message;

    message.wmcursor.position = position;

    buffer_write(message.id, 8, "content", 8, 0);
    channel_send(0, wm, MESSAGE_TYPE_WMCURSOR, sizeof (struct message_wmcursor) + 8, &message);

}

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

    channel_send(0, wm, CHANNEL_PAGE, offset, buffer);

    chunks++;

}

static void setwrap(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int offset = cstring_write_fmt(buffer, MESSAGE_SIZE, 0, "= wrap label \"%s\"\n", labels[wrap]);
    unsigned int i;

    for (i = 0; i < chunks; i++)
        offset += cstring_write_fmt(buffer, MESSAGE_SIZE, offset, "= chunk%u wrap \"%s\"\n", &i, wraps[wrap]);

    channel_send(0, wm, CHANNEL_PAGE, offset, buffer);

}

static void setbuttons(void)
{

    channel_send_fmt(0, wm, CHANNEL_PAGE, "= save mode \"%s\"\n= saveas mode \"%s\"\n", (loaded && exists) ? "normal" : "disabled", (loaded) ? "normal" : "disabled");

}

static void settitle(void)
{

    if (path[0])
        channel_send_fmt(0, wm, CHANNEL_PAGE, "= window label \"Text Editor (%s)\"\n", path + fs_dirlength(path));
    else
        channel_send_fmt(0, wm, CHANNEL_PAGE, "= window label \"Text Editor\"\n");

}

static void clearfile(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; i < chunks; i++)
        offset += cstring_write_fmt(buffer, MESSAGE_SIZE, offset, "- chunk%u\n", &i);

    if (offset)
        channel_send(0, wm, CHANNEL_PAGE, offset, buffer);

    chunks = 0;
    loaded = 0;
    exists = 0;

}

static void showfile(void)
{

    if (!wm)
        return;

    settitle();

    if (path[0])
    {

        unsigned int target = fs_auth(path);
        unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;

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

                channel_send_fmt(0, wm, CHANNEL_PAGE, "+ text id \"chunk%u\" in \"content\" weight \"bold\" label \"(only the first 16 KB are shown)\"\n", &chunks);

                chunks++;

            }

            loaded = offset < MAXSIZE;
            exists = 1;

        }

        else
        {

            channel_send_fmt(0, wm, CHANNEL_PAGE, "+ text id \"chunk%u\" in \"content\" label \"Could not open %s\"\n", &chunks, path);

            chunks++;

            setbuttons();

            return;

        }

    }

    else
    {

        loaded = 1;

    }

    channel_send_fmt(0, wm, CHANNEL_PAGE, "+ text id \"chunk%u\" in \"content\" display \"inline\" wrap \"%s\"\n", &chunks, wraps[wrap]);
    sendcursor(0);

    chunks++;

    setbuttons();

}

static unsigned int replacefile(unsigned int target, unsigned int parent, char *name, unsigned int count)
{

    unsigned int id = fs_walk(1, target, parent, name);

    if (id && !fs_remove(1, target, parent, id))
        return 0;

    id = fs_create(1, target, parent, name, cstring_length(name));

    return id && fs_write_all(1, target, id, savedata, count, 0) == count;

}

static unsigned int writefile(void)
{

    unsigned int count = 0;
    unsigned int total = 0;
    unsigned int saved = 0;
    struct message message;

    channel_send(0, wm, MESSAGE_TYPE_WMREAD, 8, "content");

    while (channel_poll(0, wm, MESSAGE_TYPE_WMTEXT, &message) && message.length)
    {

        count += buffer_write(savedata, SAVESIZE, message.data, message.length, count);
        total += message.length;

    }

    if (total == count)
    {

        char *name = path + fs_dirlength(path);
        char backup[256];
        unsigned int target;
        unsigned int parent;

        cstring_write_fmt(backup, 256, 0, "%s~\\0", name);

        target = fs_auth(path);
        parent = (target) ? fs_walkparent(1, target, path) : 0;

        if (parent && replacefile(target, parent, backup, count) && replacefile(target, parent, name, count))
        {

            unsigned int id = fs_walk(1, target, parent, backup);

            if (id)
                fs_remove(1, target, parent, id);

            saved = 1;

        }

    }

    return saved;

}

static void save(void)
{

    if (!loaded || !exists || saving)
        return;

    saving = 1;
    writefile();
    saving = 0;

}

static void saveas(char *newpath)
{

    char oldpath[256];

    if (!loaded || saving)
        return;

    saving = 1;

    cstring_write_fmt(oldpath, 256, 0, "%s\\0", path);
    cstring_write_fmt(path, 256, 0, "%s\\0", newpath);

    if (writefile())
        exists = 1;
    else
        cstring_write_fmt(path, 256, 0, "%s\\0", oldpath);

    saving = 0;

    settitle();
    setbuttons();

}

static void choose(char *mode)
{

    unsigned int target = fs_spawn(1, "initrd:bin/wfile");

    if (target)
    {

        struct message_pipe pipe;

        choosing = (cstring_match(mode, "save")) ? CHOOSE_SAVE : CHOOSE_OPEN;

        channel_send_fmt(0, target, MESSAGE_TYPE_OPTION, "mode\\0%s\\0wm-service\\0%s\\0", mode, option_getstring("wm-service"));
        message_initpipe(&pipe);
        channel_send(0, target, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
        channel_send(0, target, MESSAGE_TYPE_TERM, 0, 0);

    }

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wedit.alfi";

        channel_send(0, wm, MESSAGE_TYPE_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        showfile();
        channel_hold(0);
        channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);

    }

}

static void onpath(struct message *message)
{

    if (saving || (path[0] && !choosing))
        return;

    if (choosing == CHOOSE_SAVE)
    {

        choosing = 0;

        saveas(message->data);

        return;

    }

    choosing = 0;

    clearfile();
    cstring_write_fmt(path, 256, 0, "%s\\0", message->data);
    showfile();

}

static void onwmevent(struct message *message)
{

    struct message_wmevent *event = message_validate(MESSAGE_TYPE_WMEVENT, message->data, message->length);

    if (event)
    {

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
            choose("select");

        if (kv_match(event, "q=saveas"))
            choose("save");

        if (kv_match(event, "q=save"))
            save();

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_WMEVENT, onwmevent);

}

