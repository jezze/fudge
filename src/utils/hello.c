#include <fudge.h>
#include <abi.h>

void main(void)
{

    struct channel channel;
    unsigned int id;

    channel_init(&channel);
    channel_listen(&channel);

    id = channel_reply(&channel, EVENT_DATA);

    channel_append(&channel, 13, "Hello world!\n");
    channel_place(&channel, id);

}

