#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static unsigned int dirlength(char *path)
{

    unsigned int length = 0;
    unsigned int i;

    for (i = 0; path[i]; i++)
    {

        if (path[i] == '/' || path[i] == ':')
            length = i + 1;

    }

    return length;

}

static unsigned int walkdirectory(unsigned int target, char *path)
{

    char directory[1024];
    unsigned int length = dirlength(path);

    if (length >= 1024)
        return 0;

    buffer_write(directory, 1024, path, length, 0);

    directory[length] = '\0';

    return fs_walk(1, target, 0, directory);

}

static void touch(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);
    char *name = path + dirlength(path);
    unsigned int parent;

    if (!target)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    if (fs_walk(1, target, 0, path))
        return;

    parent = walkdirectory(target, path);

    if (!parent)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    if (!cstring_length(name) || !fs_create(1, target, parent, name, cstring_length(name)))
        channel_send_fmt1(0, source, EVENT_ERROR, "File could not be created: %s\n", path);

}

static void onpath(struct message *message)
{

    paths++;

    touch(message->source, message->data);

}

static void onterm(struct message *message)
{

    if (!paths)
        channel_send_fmt0(0, message->source, EVENT_ERROR, "Usage: touch <path>\n");

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}
