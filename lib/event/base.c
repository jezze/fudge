#include <abi.h>
#include <fudge.h>
#include "base.h"

unsigned int event_read(unsigned int descriptor, struct event_header *header)
{

    while (file_readall(descriptor, header, sizeof (struct event_header)) != sizeof (struct event_header));
    while (file_readall(descriptor, header + 1, header->length - sizeof (struct event_header)) != header->length - sizeof (struct event_header));

    return header->length;

}

unsigned int event_send(unsigned int descriptor, struct event_header *header, unsigned int type, unsigned int source, unsigned int destination, unsigned int length)
{

    event_initheader(header, type, source, destination, length);

    return file_writeall(descriptor, header, header->length);

}

void event_sendinit(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_INIT, source, destination, 0);

}

void event_sendexit(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_EXIT, source, destination, 0);

}

void event_sendkill(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_KILL, source, destination, 0);

}

void event_senddata(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int count, void *buffer)
{

    struct {struct event_header header; char data[FUDGE_BSIZE];} message;

    event_send(descriptor, &message.header, EVENT_DATA, source, destination, memory_write(message.data, FUDGE_BSIZE, buffer, count, 0));

}

void event_sendfile(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned char num)
{

    struct {struct event_header header; struct event_file file;} message;

    message.file.num = num;

    event_send(descriptor, &message.header, EVENT_FILE, source, destination, sizeof (struct event_file));

}

void event_sendchild(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_CHILD, source, destination, 0);

}

