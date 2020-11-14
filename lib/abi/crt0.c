#include <fudge.h>
#include <abi.h>

extern void init(struct channel *channel);

void main(void)
{

    struct channel channel;
    struct message_header header;
    struct message_data data;

    channel_init(&channel);
    init(&channel);
    channel_pollall(&channel, &header, &data);

}

