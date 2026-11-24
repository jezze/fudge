#include <fudge.h>
#include <abi.h>

static void ondata(struct message *message)
{

    channel_send(0, message->source, message->type, message->length, message->data);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_INFO, ondata);

}

