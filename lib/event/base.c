#include <abi.h>
#include <fudge.h>
#include "base.h"

void *event_getforward(void *buffer)
{

    struct event_header *header = buffer;

    return header + 1;

}

void *event_getdata(void *buffer)
{

    struct event_header *header = buffer;
    struct event_forward *forward = event_getforward(buffer);

    return forward + header->forward;

}

unsigned int event_addheader(void *buffer, unsigned int type, unsigned int source, unsigned int target)
{

    struct event_header *header = buffer;

    header->type = type;
    header->source = source;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->forward = 0;

    return header->length;

}

unsigned int event_addforward(void *buffer, unsigned int target)
{

    struct event_header *header = buffer;
    struct event_forward *forward = event_getforward(buffer);

    forward[header->forward].target = target;
    header->forward++;
    header->length += sizeof (struct event_forward);

    return header->length;

}

unsigned int event_addrequest(void *buffer, struct event_header *header, unsigned int type, unsigned int id)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->target, id);

    if (header->forward)
    {

        struct event_forward *forward = event_getforward(header);
        unsigned int i;

        for (i = 0; i < header->forward; i++)
            event_addforward(buffer, forward[i].target);

    }

    event_addforward(buffer, header->target);

    return reply->length;

}

unsigned int event_addpipe(void *buffer, struct event_header *header, unsigned int type, unsigned int id)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->target, id);

    if (header->forward)
    {

        struct event_forward *forward = event_getforward(header);
        unsigned int i;

        for (i = 0; i < header->forward; i++)
            event_addforward(buffer, forward[i].target);

    }

    return reply->length;

}

unsigned int event_addresponse(void *buffer, struct event_header *header, unsigned int type)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->target, header->source);

    if (header->forward)
    {

        struct event_forward *forward = event_getforward(header);
        unsigned int i;

        for (i = 0; i < header->forward - 1; i++)
            event_addforward(buffer, forward[i].target);

        reply->target = forward[header->forward - 1].target;

    }

    return reply->length;

}

unsigned int event_adddata(void *buffer, unsigned int count, void *data)
{

    struct event_header *header = buffer;
    struct event_data *d = event_getdata(buffer);

    header->length += sizeof (struct event_data);
    d->count = memory_write(buffer, FUDGE_BSIZE, data, count, header->length);
    header->length += d->count;

    return header->length;

}

unsigned int event_addfile(void *buffer, unsigned int descriptor)
{

    struct event_header *header = buffer;
    struct event_file *file = event_getdata(buffer);

    header->length += sizeof (struct event_file);
    file->descriptor = descriptor;

    return header->length;

}

void event_open(void)
{

    file_walk(FILE_PM, "/system/event");
    file_open(FILE_PM);

}

void event_close(void)
{

    file_close(FILE_PM);

}

struct event_header *event_read(void *data)
{

    struct event_header *header = data;

    while (file_readall(FILE_PM, header, sizeof (struct event_header)) != sizeof (struct event_header));
    while (file_readall(FILE_PM, header + 1, header->length - sizeof (struct event_header)) != header->length - sizeof (struct event_header));

    return header;

}

unsigned int event_send(void *buffer)
{

    struct event_header *message = buffer;

    return file_writeall(FILE_PM, message, message->length);

}

