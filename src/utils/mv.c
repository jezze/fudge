#include <fudge.h>
#include <abi.h>

static char source[1024];
static unsigned int paths;

static char *basename(char *path)
{

    char *name = path;
    unsigned int i;

    for (i = 0; path[i]; i++)
    {

        if (path[i] == '/' || path[i] == ':')
            name = path + i + 1;

    }

    return name;

}

static void move(unsigned int source, char *from, char *to)
{

    unsigned int starget = fs_auth(from);
    unsigned int dtarget = fs_auth(to);
    unsigned int sid = (starget) ? fs_walk(1, starget, 0, from) : 0;
    unsigned int did = (dtarget) ? fs_walk(1, dtarget, 0, to) : 0;
    char *name = (cstring_length(option_getstring("name"))) ? option_getstring("name") : basename(from);
    char buffer[0x800];
    unsigned int offset = 0;
    unsigned int count;
    unsigned int id;

    if (!sid)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", from);

        return;

    }

    if (!did)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", to);

        return;

    }

    id = fs_create(1, dtarget, did, name, cstring_length(name));

    if (!id)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "File could not be created: %s\n", name);

        return;

    }

    while ((count = fs_read(1, starget, sid, buffer, 0x800, offset)))
    {

        unsigned int written = 0;

        while (written < count)
        {

            unsigned int n = fs_write(1, dtarget, id, buffer + written, count - written, offset + written);

            if (!n)
            {

                channel_send_fmt1(0, source, EVENT_ERROR, "File could not be written: %s\n", name);

                return;

            }

            written += n;

        }

        offset += count;

    }

    if (!fs_remove(1, starget, sid))
        channel_send_fmt1(0, source, EVENT_ERROR, "File could not be removed: %s\n", from);

}

static void onpath(struct message *message)
{

    if (!paths)
        buffer_write(source, 1024, message->data, cstring_length_zero(message->data), 0);
    else if (paths == 1)
        move(message->source, source, message->data);

    paths++;

}

static void onterm(struct message *message)
{

    if (paths != 2)
        channel_send_fmt0(0, message->source, EVENT_ERROR, "Usage: mv <source> <directory>\n");

}

void init(void)
{

    option_add("name", "");
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}
