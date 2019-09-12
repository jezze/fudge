#include <fudge.h>
#include <abi.h>

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_listen(&channel);
    channel_reply(&channel, EVENT_DATA);
    channel_append(&channel, 13, "Hello world!\n");
    channel_place(&channel, channel.o.header.target);

}

