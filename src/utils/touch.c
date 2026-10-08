#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void touch(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);
    char *name = path + fs_dirlength(path);
    unsigned int parent;

    if (!target)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    if (fs_walk(1, target, 0, path))
        return;

    parent = fs_walkparent(1, target, path);

    if (!parent)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    if (!cstring_length(name) || !fs_create(1, target, parent, name, cstring_length(name)))
        channel_send_fmt(0, source, EVENT_ERROR, "File could not be created: %s\n", path);

}

static void onpath(struct message *message)
{

    paths++;

    touch(message->source, message->data);

}

static void onterm(struct message *message)
{

    if (!paths)
        channel_send_fmt(0, message->source, EVENT_ERROR, "Usage: touch <path>\n");

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}
