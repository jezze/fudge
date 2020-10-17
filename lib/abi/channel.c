#include <fudge.h>
#include "call.h"
#include "file.h"
#include "channel.h"

static void dispatch(struct channel *channel, struct message_header *header, void *data)
{

    if (header->type < EVENTS)
    {

        if (channel->signals[header->type].callback)
            channel->signals[header->type].callback(channel, header->source, data, message_datasize(header));

        if (channel->signals[EVENT_ANY].callback)
            channel->signals[EVENT_ANY].callback(channel, header->source, data, message_datasize(header));

    }

}

static unsigned int poll(struct channel *channel, struct message_header *header, void *data)
{

    while (channel->poll)
    {

        if (call_pick(header, data))
            return header->type;

    }

    return 0;

}

unsigned int channel_place(struct channel *channel, union message *message, unsigned int id)
{

    if (message->header.type < EVENTS)
    {

        if (channel->signals[message->header.type].redirect)
            id = channel->signals[message->header.type].redirect;

        return call_place(id, &message->header, message->data + sizeof (struct message_header));

    }

    return 0;

}

unsigned int channel_write(struct channel *channel, union message *message, unsigned int descriptor)
{

    return file_writeall(descriptor, message, message->header.length);

}

unsigned int channel_poll(struct channel *channel, struct message_header *header, void *data, unsigned int type)
{

    while (poll(channel, header, data))
    {

        if (header->type == type)
            return type;

        dispatch(channel, header, data);

    }

    return 0;

}

void channel_listen(struct channel *channel)
{

    struct message_header header;
    char data[FUDGE_BSIZE];

    channel->poll = 1;

    while (poll(channel, &header, data))
        dispatch(channel, &header, data);

}

void channel_listen2(struct channel *channel, void (*oninit)(struct channel *channel), void (*onexit)(struct channel *channel))
{

    if (oninit)
        oninit(channel);

    channel_listen(channel);

    if (onexit)
        onexit(channel);

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

void channel_init(struct channel *channel)
{

    unsigned int i;

    for (i = 0; i < EVENTS; i++)
    {

        channel_setsignal(channel, i, 0);
        channel_setredirect(channel, i, 0, 0, 0);

    }

}

void channel_senddata(struct channel *channel, unsigned int id, unsigned int count, void *data)
{

    union message message;

    message_init(&message, EVENT_DATA);
    message_append(&message, count, data);
    channel_place(channel, &message, id);

}

void channel_sendfile(struct channel *channel, unsigned int id, char *path)
{

    union message message;

    message_init(&message, EVENT_FILE);
    message_appendstring2(&message, path);
    channel_place(channel, &message, id);

}

void channel_sendmain(struct channel *channel, unsigned int id)
{

    union message message;

    message_init(&message, EVENT_MAIN);
    channel_place(channel, &message, id);

}

void channel_sendredirect(struct channel *channel, unsigned int id, unsigned int type, unsigned int mode, unsigned int source)
{

    struct event_redirect redirect;
    union message message;

    redirect.type = type;
    redirect.mode = mode;
    redirect.id = source;

    message_init(&message, EVENT_REDIRECT);
    message_append(&message, sizeof (struct event_redirect), &redirect);
    channel_place(channel, &message, id);

}

