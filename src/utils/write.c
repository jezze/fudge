#include <fudge.h>
#include <abi.h>

static unsigned int target;
static unsigned int id;
static unsigned int offset;

static void ondata(struct message *message)
{

    if (id)
        offset += fs_write_all(1, target, id, message->data, message->length, offset);

}

static void onpath(struct message *message)
{

    target = fs_auth(message->data);
    id = (target) ? fs_walk(1, target, 0, message->data) : 0;
    offset = 0;

    if (!id)
        channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

