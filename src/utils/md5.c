#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct md5 sum;

static void ondata(struct message *message)
{

    md5_read(&sum, message->data, message->length);

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
                md5_read(&sum, buffer, count);

        }

        else
        {

            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

}

static void onterm(struct message *message)
{

    unsigned char digest[16];
    char output[32];
    unsigned int l = 32;
    unsigned int i;

    md5_write(&sum, digest);

    for (i = 0; i < 16; i++)
        cstring_write_value(output, 32, digest[i], 16, 2, i * 2);

    channel_send_fmt2(0, message->source, EVENT_DATA, "%w\n", output, &l);

}

void init(void)
{

    md5_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

    while (channel_process(0));

}

