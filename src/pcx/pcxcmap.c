#include <fudge.h>
#include <abi.h>
#include <image.h>

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);
    unsigned int id = fs_walk(1, target, 0, message->data);

    if (id)
    {

        struct pcx_header header;
        struct record record;
        unsigned char magic;

        fs_stat(1, target, id, &record);
        fs_read_all(1, target, id, &header, sizeof (struct pcx_header), 0);
        fs_read_all(1, target, id, &magic, 1, record.size - 768 - 1);

        if (magic == PCX_COLORMAP_MAGIC)
        {

            unsigned char colormap[768];

            fs_read_all(1, target, id, colormap, 768, record.size - 768);
            channel_send(0, message->source, EVENT_DATA, 768, colormap);

        }

    }

    else
    {

        channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

