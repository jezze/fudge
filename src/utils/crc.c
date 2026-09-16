#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct crc sum;

static void ondata(struct message *message)
{

    crc_read(&sum, message->data, message->length);

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
        {

            char buffer[4096];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, 4096, offset)); offset += count)
                crc_read(&sum, buffer, count);

        }

        else
        {

            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

}

static void onterm(struct message *message)
{

    unsigned int crc = crc_finalize(&sum);

    channel_send_fmt1(0, message->source, EVENT_DATA, "%u\n", &crc);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

    while (channel_process(0));

}

