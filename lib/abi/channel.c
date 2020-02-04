#include <fudge.h>
#include "call.h"
#include "file.h"
#include "channel.h"

unsigned int channel_place(struct channel *channel, unsigned int id)
{

    if (channel->signals[channel->message.header.type].redirect)
        id = channel->signals[channel->message.header.type].redirect;

    return call_place(id, &channel->message.header, channel->message.data);

}

unsigned int channel_write(struct channel *channel, unsigned int descriptor)
{

    return file_writeall(descriptor, &channel->message, channel->message.header.length);

}

void channel_dispatch(struct channel *channel, struct ipc_header *header, void *data)
{

    if (channel->signals[header->type].callback)
        channel->signals[header->type].callback(channel, header->source, data, ipc_datasize(header));

    if (channel->signals[EVENT_ANY].callback)
        channel->signals[EVENT_ANY].callback(channel, header->source, data, ipc_datasize(header));

}

void channel_listen(struct channel *channel)
{

    channel->poll = 1;

    while (channel->poll)
    {

        struct ipc_header header;
        char data[FUDGE_BSIZE];

        if (call_pick(&header, data))
            channel_dispatch(channel, &header, data);

    }

}

void channel_listen2(struct channel *channel, void (*oninit)(struct channel *channel), void (*onexit)(struct channel *channel))
{

    oninit(channel);
    channel_listen(channel);
    onexit(channel);

}

unsigned int channel_listenfor(struct channel *channel, unsigned int type, struct ipc_header *header, void *data)
{

    channel->poll = 1;

    while (channel->poll)
    {

        if (call_pick(header, data))
        {

            if (header->type == type)
                return type;
            else
                channel_dispatch(channel, header, data);

        }

    }

    return 0;

}

void channel_close(struct channel *channel)
{

    channel->poll = 0;

}

void channel_setredirect(struct channel *channel, unsigned int type, unsigned int mode, unsigned int id, unsigned int source)
{

    switch (mode)
    {

    case 1:
        channel->signals[type].redirect = id;

        break;

    case 2:
        channel->signals[type].redirect = source;

        break;

    default:
        channel->signals[type].redirect = 0;

        break;

    }

}

void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, unsigned int source, void *mdata, unsigned int msize))
{

    channel->signals[type].callback = callback;

}

void channel_request(struct channel *channel, unsigned int type)
{

    ipc_init(&channel->message.header, type, 0);

}

void channel_append(struct channel *channel, unsigned int count, void *buffer)
{

    channel->message.header.length += memory_write(channel->message.data, FUDGE_BSIZE - sizeof (struct ipc_header) - 1, buffer, count, ipc_datasize(&channel->message.header));

}

void channel_appendstring(struct channel *channel, char *string)
{

    channel->message.header.length += memory_write(channel->message.data, FUDGE_BSIZE - sizeof (struct ipc_header) - 1, string, ascii_length(string), ipc_datasize(&channel->message.header));

}

void channel_appendvalue(struct channel *channel, int value, unsigned int base, unsigned int padding)
{

    char num[FUDGE_NSIZE];

    channel->message.header.length += memory_write(channel->message.data, FUDGE_BSIZE - sizeof (struct ipc_header) - 1, num, ascii_wvalue(num, FUDGE_NSIZE, value, base, padding), ipc_datasize(&channel->message.header));

}

void channel_init(struct channel *channel)
{

    unsigned int i;

    for (i = 0; i < EVENTS; i++)
    {

        channel_setsignal(channel, i, 0);
        channel_setredirect(channel, i, 0, 0, 0);

    }

}

