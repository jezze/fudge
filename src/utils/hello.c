#include <fudge.h>
#include <abi.h>

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_listen(&channel);
    channel_reply(&channel, EVENT_DATA);
    event_append(&channel.o.header, 13, "Hello world!\n");
    channel_place(channel.o.header.target, &channel.o);

}

