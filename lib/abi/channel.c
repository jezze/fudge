#include <fudge.h>
#include "call.h"
#include "file.h"
#include "channel.h"

unsigned int channel_pick(struct channel *channel, void *data)
{

    return call_pick(&channel->i, data);

}

unsigned int channel_place(struct channel *channel, unsigned int id)
{

    return call_place(id, &channel->message.header, channel->message.data);

}

unsigned int channel_write(struct channel *channel, unsigned int descriptor)
{

    return file_writeall(descriptor, &channel->message, channel->message.header.length);

}

void channel_listen(struct channel *channel)
{

    channel->poll = 1;

    while (channel->poll)
    {

        char data[FUDGE_BSIZE];

        if (channel_pick(channel, data))
        {

            if (channel->signals[channel->i.type])
                channel->signals[channel->i.type](channel, data, channel->i.length - sizeof (struct ipc_header));

            if (channel->signals[EVENT_ANY])
                channel->signals[EVENT_ANY](channel, data, channel->i.length - sizeof (struct ipc_header));

        }

    }

}

void channel_close(struct channel *channel)
{

    unsigned int id = channel_reply(channel, EVENT_DONE);

    channel_place(channel, id);

    channel->poll = 0;

}

void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, void *mdata, unsigned int msize))
{

    channel->signals[type] = callback;

}

void channel_forward(struct channel *channel, unsigned int type)
{

    unsigned int i;

    ipc_create(&channel->message.header, type, 0);

    for (i = 0; i < channel->i.nroutes; i++)
        ipc_addroute(&channel->message.header, channel->i.routes[i]);

}

void channel_request(struct channel *channel, unsigned int type)
{

    channel_forward(channel, type);
    ipc_addroute(&channel->message.header, channel->i.target);

}

unsigned int channel_reply(struct channel *channel, unsigned int type)
{

    channel_forward(channel, type);

    if (channel->message.header.nroutes)
    {

        channel->message.header.nroutes--;

        return channel->message.header.routes[channel->message.header.nroutes];

    }

    return channel->i.source;

}

void channel_append(struct channel *channel, unsigned int count, void *buffer)
{

    channel->message.header.length += memory_write(channel->message.data, FUDGE_BSIZE - sizeof (struct ipc_header) - 1, buffer, count, channel->message.header.length - sizeof (struct ipc_header));

}

void channel_appendstring(struct channel *channel, char *string)
{

    channel->message.header.length += memory_write(channel->message.data, FUDGE_BSIZE - sizeof (struct ipc_header) - 1, string, ascii_length(string), channel->message.header.length - sizeof (struct ipc_header));

}

void channel_appendvalue(struct channel *channel, int value, unsigned int base, unsigned int padding)
{

    char num[FUDGE_NSIZE];

    channel->message.header.length += memory_write(channel->message.data, FUDGE_BSIZE - sizeof (struct ipc_header) - 1, num, ascii_wvalue(num, FUDGE_NSIZE, value, base, padding), channel->message.header.length - sizeof (struct ipc_header));

}

void channel_init(struct channel *channel)
{

    unsigned int i;

    for (i = 0; i < EVENTS; i++)
        channel_setsignal(channel, i, 0);

    ipc_create(&channel->i, 0, 0);
    ipc_create(&channel->message.header, 0, 0);

}

