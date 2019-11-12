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

void channel_listen(struct channel *channel)
{

    channel->poll = 1;

    while (channel->poll)
    {

        struct ipc_header header;
        char data[FUDGE_BSIZE];

        if (call_pick(&header, data))
        {

            if (channel->signals[header.type].callback)
                channel->signals[header.type].callback(channel, header.source, data, header.length - sizeof (struct ipc_header));

            if (channel->signals[EVENT_ANY].callback)
                channel->signals[EVENT_ANY].callback(channel, header.source, data, header.length - sizeof (struct ipc_header));

        }

    }

}

void channel_listen2(struct channel *channel, void (*oninit)(struct channel *channel), void (*onexit)(struct channel *channel))
{

    oninit(channel);
    channel_listen(channel);
    onexit(channel);

}

void channel_close(struct channel *channel)
{

    channel->poll = 0;

}

void channel_setredirect(struct channel *channel, unsigned int type, unsigned int id, unsigned int source)
{

    channel->signals[type].redirect = (id == 255) ? source : id;

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
    {

        channel_setsignal(channel, i, 0);
        channel_setredirect(channel, i, 0, 0);

    }

}

