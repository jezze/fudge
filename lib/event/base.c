#include <abi.h>
#include <fudge.h>
#include "base.h"

void *event_getheader(void *buffer)
{

    return buffer;

}

void *event_getforward(struct event_header *header)
{

    return header + 1;

}

void *event_getdata(struct event_header *header)
{

    struct event_forward *forward = event_getforward(header);

    return forward + header->forward;

}

void *event_addheader(void *buffer, unsigned int type, unsigned int source, unsigned int target)
{

    struct event_header *header = buffer;

    header->type = type;
    header->source = source;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->forward = 0;

    return header + 1;

}

void *event_addforward(void *buffer, unsigned int target)
{

    struct event_header *header = event_getheader(buffer);
    struct event_forward *forward = event_getforward(header);

    forward[header->forward].target = target;
    header->forward++;
    header->length += sizeof (struct event_forward);

    return forward + header->forward;

}

struct event_header *event_read(unsigned int descriptor, void *data)
{

    struct event_header *header = data;

    while (file_readall(descriptor, header, sizeof (struct event_header)) != sizeof (struct event_header));
    while (file_readall(descriptor, header + 1, header->length - sizeof (struct event_header)) != header->length - sizeof (struct event_header));

    return header;

}

unsigned int event_send(unsigned int descriptor, struct event_header *header, unsigned int type, unsigned int source, unsigned int target, unsigned int length)
{

    event_initheader(header, type, source, target, length);

    return file_writeall(descriptor, header, header->length);

}

unsigned int event_sendinit(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_INIT, source, target, 0);

}

unsigned int event_sendexit(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_EXIT, source, target, 0);

}

unsigned int event_sendkill(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_KILL, source, target, 0);

}

unsigned int event_senddata(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int count, void *buffer)
{

    struct {struct event_header header; struct event_data data; char buffer[0x800];} message;

    message.data.count = memory_write(message.buffer, 0x800, buffer, count, 0);

    return event_send(descriptor, &message.header, EVENT_DATA, source, target, sizeof (struct event_data) + message.data.count);

}

unsigned int event_sendfile(unsigned int descriptor, unsigned int source, unsigned int target, unsigned char num)
{

    struct {struct event_header header; struct event_file file;} message;

    message.file.num = num;

    return event_send(descriptor, &message.header, EVENT_FILE, source, target, sizeof (struct event_file));

}

unsigned int event_sendchild(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_CHILD, source, target, 0);

}

