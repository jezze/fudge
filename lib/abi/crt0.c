#include <fudge.h>
#include <abi.h>

extern void init(struct channel *channel);

static struct channel_callback callbacks[CHANNEL_CALLBACKS];

void main(void)
{

    struct channel channel;
    struct message_header header;
    struct message_data data;

    channel_init(&channel, callbacks);
    init(&channel);

    while (channel_poll(&channel, &header, &data))
        channel_dispatch(&channel, &header, &data);

}

