#include <fudge.h>
#include <abi.h>
#include <binary.h>

static char mapdata[4096];
static unsigned int mapcount;
static unsigned int width;

static unsigned int marksymbols(char from, char to)
{

    unsigned int offset = 0;
    unsigned int total = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(mapdata, mapcount, '\n', offset)); i = offset)
    {

        if (mapdata[i + width + 1] == from)
        {

            mapdata[i + width + 1] = to;

            total++;

        }

    }

    return total;

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
        {

            char mapname[256];

            cstring_write_fmt(mapname, 256, 0, "%s.map\\0", message->data);

            mapcount = fs_read_path(1, mapname, mapdata, 4096);

            if (!marksymbols('L', 'X'))
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Not loaded: %s\n", message->data);
            else if (call_unload(target, id) == MESSAGE_OK)
                fs_write_all(1, target, fs_walk(1, target, 0, mapname), mapdata, mapcount, 0);

        }

        else
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Path not found: %s\n", message->data);

        }

    }

}

void init(void)
{

    width = sizeof (void *) * 2;

    channel_bind(MESSAGE_TYPE_PATH, onpath);

}

