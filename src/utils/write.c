#include <fudge.h>
#include <abi.h>

static unsigned int target;
static unsigned int id;
static unsigned int offset;

static unsigned int event;

static void store(unsigned int source, void *data, unsigned int count)
{

    if (id)
    {

        unsigned int written = fs_write_all(1, target, id, data, count, offset);

        offset += written;

        if (written < count)
        {

            id = 0;

            channel_send_fmt(0, source, EVENT_ERROR, "File could not be written\n");

        }

    }

}

static void ondata(struct message *message)
{

    store(message->source, message->data, message->length);

}

static void onrecord(struct message *message)
{

    if (message->event == EVENT_ERROR)
    {

        store(message->source, message->data, message->length);

    }

    else
    {

        char data[MESSAGE_SIZE + 8];
        unsigned int *header = (unsigned int *)data;

        header[0] = message->event;
        header[1] = message->length;

        store(message->source, data, 8 + buffer_write(data, MESSAGE_SIZE + 8, message->data, message->length, 8));

    }

}

static void onmain(struct message *message)
{

    event = option_getdecimal("event");

    if (!event || event == EVENT_DATA)
        return;

    switch (event)
    {

    case EVENT_MAIN:
    case EVENT_OPTION:
    case EVENT_PATH:
    case EVENT_PIPE:
        channel_send_fmt(0, message->source, EVENT_ERROR, "Event can not be recorded\n");

        break;

    default:
        channel_bind(event, onrecord);

        break;

    }

}

static void onpath(struct message *message)
{

    char *path = message->data;
    char *name = path + fs_dirlength(path);
    unsigned int create = option_getdecimal("create");
    unsigned int append = option_getdecimal("append");
    unsigned int parent;

    target = fs_auth(path);
    id = (target) ? fs_walk(1, target, 0, path) : 0;
    offset = 0;

    if (!create && !append)
    {

        if (!id)
            channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    parent = (target) ? fs_walkparent(1, target, path) : 0;

    if (!parent)
    {

        id = 0;

        channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    if (id && append)
    {

        struct record record;

        if (fs_stat(1, target, id, &record))
            offset = record.size;

        return;

    }

    if (id && !fs_remove(1, target, parent, id))
    {

        id = 0;

        channel_send_fmt(0, message->source, EVENT_ERROR, "File could not be replaced: %s\n", path);

        return;

    }

    id = (cstring_length(name)) ? fs_create(1, target, parent, name, cstring_length(name)) : 0;

    if (!id)
        channel_send_fmt(0, message->source, EVENT_ERROR, "File could not be created: %s\n", path);

}

void init(void)
{

    option_add("create", "0");
    option_add("append", "0");
    option_add("event", "0");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}
