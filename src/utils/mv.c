#include <fudge.h>
#include <abi.h>

static char source[1024];
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

static unsigned int isdirectory(unsigned int target, unsigned int id)
{

    struct record record;

    return fs_stat(1, target, id, &record) && record.type == RECORD_TYPE_DIRECTORY;

}

static unsigned int move(unsigned int source, char *from, char *to)
{

    unsigned int starget = fs_auth(from);
    unsigned int dtarget = fs_auth(to);
    unsigned int sid = (starget) ? fs_walk(1, starget, 0, from) : 0;
    unsigned int did = (dtarget) ? fs_walk(1, dtarget, 0, to) : 0;
    unsigned int parent;
    char *name;
    char buffer[0x800];
    unsigned int offset = 0;
    unsigned int count;
    unsigned int id;

    if (!sid)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", from);

        return 0;

    }

    if (did && isdirectory(dtarget, did))
    {

        parent = did;
        name = from + dirlength(from);

    }

    else
    {

        parent = (dtarget) ? walkdirectory(dtarget, to) : 0;
        name = to + dirlength(to);

    }

    if (!parent)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", to);

        return 0;

    }

    if (did && !isdirectory(dtarget, did))
    {

        if (dtarget == starget && did == sid)
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "Same file: %s\n", from);

            return 0;

        }

        if (!fs_remove(1, dtarget, parent, did))
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "File could not be replaced: %s\n", to);

            return 0;

        }

    }

    id = fs_create(1, dtarget, parent, name, cstring_length(name));

    if (!id)
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "File could not be created: %s\n", name);

        return 0;

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

                return 0;

            }

            written += n;

        }

        offset += count;

    }

    if (!fs_remove(1, starget, walkdirectory(starget, from), sid))
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "File could not be removed: %s\n", from);

        return 0;

    }

    return 1;

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
        channel_send_fmt0(0, message->source, EVENT_ERROR, "Usage: mv <source> <destination>\n");

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}
