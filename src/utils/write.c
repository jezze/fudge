#include <fudge.h>
#include <abi.h>

static unsigned int target;
static unsigned int id;
static unsigned int offset;

static void ondata(struct message *message)
{

    if (id)
    {

        unsigned int count = fs_write_all(1, target, id, message->data, message->length, offset);

        offset += count;

        if (count < message->length)
        {

            id = 0;

            channel_send_fmt0(0, message->source, EVENT_ERROR, "File could not be written\n");

        }

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
            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    parent = (target) ? fs_walkparent(1, target, path) : 0;

    if (!parent)
    {

        id = 0;

        channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", path);

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

        channel_send_fmt1(0, message->source, EVENT_ERROR, "File could not be replaced: %s\n", path);

        return;

    }

    id = (cstring_length(name)) ? fs_create(1, target, parent, name, cstring_length(name)) : 0;

    if (!id)
        channel_send_fmt1(0, message->source, EVENT_ERROR, "File could not be created: %s\n", path);

}

void init(void)
{

    option_add("create", "0");
    option_add("append", "0");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}
