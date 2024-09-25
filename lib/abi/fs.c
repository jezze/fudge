#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"

static unsigned int sessioncount;

unsigned int fs_auth(char *path)
{

    return call_find(cstring_length(path), path);

}

unsigned int fs_list(unsigned int target, unsigned int id, unsigned int offset, struct record *records, unsigned int nrecords)
{

    struct event_listrequest request;
    unsigned char data[MESSAGE_SIZE];
    struct message message;

    request.session = ++sessioncount;
    request.id = id;
    request.offset = offset;
    request.nrecords = nrecords;

    channel_send_buffer(target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &request);

    while (channel_poll(target, EVENT_LISTRESPONSE, &message, MESSAGE_SIZE, data))
    {

        struct event_listresponse *response = (void *)data;

        if (response->session == request.session)
            return buffer_write(records, sizeof (struct record) * nrecords, response + 1, sizeof (struct record) * response->nrecords, 0) / sizeof (struct record);

    }

    return 0;

}

unsigned int fs_map(unsigned int target, unsigned int id)
{

    struct event_maprequest request;
    unsigned char data[MESSAGE_SIZE];
    struct message message;

    request.session = ++sessioncount;
    request.id = id;

    channel_send_buffer(target, EVENT_MAPREQUEST, sizeof (struct event_maprequest), &request);

    while (channel_poll(target, EVENT_MAPRESPONSE, &message, MESSAGE_SIZE, data))
    {

        struct event_mapresponse *response = (void *)data;

        if (response->session == request.session)
            return response->address;

    }

    return 0;

}

unsigned int fs_read(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_readrequest request;
    unsigned char data[MESSAGE_SIZE];
    struct message message;

    request.session = ++sessioncount;
    request.id = id;
    request.offset = offset;
    request.count = count;

    channel_send_buffer(target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request);

    while (channel_poll(target, EVENT_READRESPONSE, &message, MESSAGE_SIZE, data))
    {

        struct event_readresponse *response = (void *)data;

        if (response->session == request.session)
            return buffer_write(buffer, count, response + 1, response->count, 0);

    }

    return 0;

}

unsigned int fs_read_full(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *rbuffer = buffer;
    unsigned int rtotal = 0;
    unsigned int roffset;
    unsigned int rcount;

    for (roffset = offset; (rcount = fs_read(target, id, rbuffer + rtotal, count - rtotal, roffset)); roffset += rcount)
        rtotal += rcount;

    return rtotal;

}

unsigned int fs_read_all(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += fs_read(target, id, b + c, count - c, offset + c));

    return c;

}

unsigned int fs_stat(unsigned int target, unsigned int id, struct record *record)
{

    struct event_statrequest request;
    unsigned char data[MESSAGE_SIZE];
    struct message message;

    request.session = ++sessioncount;
    request.id = id;

    channel_send_buffer(target, EVENT_STATREQUEST, sizeof (struct event_statrequest), &request);

    while (channel_poll(target, EVENT_STATRESPONSE, &message, MESSAGE_SIZE, data))
    {

        struct event_statresponse *response = (void *)data;

        if (response->session == request.session)
            return buffer_write(record, sizeof (struct record), response + 1, sizeof (struct record) * response->nrecords, 0) / sizeof (struct record);

    }

    return 0;

}

unsigned int fs_walk(unsigned int target, unsigned int parent, char *path)
{

    struct {struct event_walkrequest header; char path[64];} request;
    unsigned char data[MESSAGE_SIZE];
    struct message message;

    request.header.session = ++sessioncount;
    request.header.parent = parent;
    request.header.length = buffer_write(request.path, 64, path, cstring_length(path), 0);

    channel_send_buffer(target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest), &request);

    while (channel_poll(target, EVENT_WALKRESPONSE, &message, MESSAGE_SIZE, data))
    {

        struct event_walkresponse *response = (void *)data;

        if (response->session == request.header.session)
            return response->id;

    }

    return 0;

}

unsigned int fs_write(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct {struct event_writerequest header; char data[64];} request;
    unsigned char data[MESSAGE_SIZE];
    struct message message;

    request.header.session = ++sessioncount;
    request.header.id = id;
    request.header.offset = offset;
    request.header.count = buffer_write(request.data, 64, buffer, count, 0);

    channel_send_buffer(target, EVENT_WRITEREQUEST, sizeof (struct event_writerequest), &request);

    while (channel_poll(target, EVENT_WRITERESPONSE, &message, MESSAGE_SIZE, data))
    {

        struct event_writeresponse *response = (void *)data;

        if (response->session == request.header.session)
            return buffer_write(buffer, count, response + 1, response->count, 0);

    }

    return 0;

}

unsigned int fs_write_all(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += fs_write(target, id, b + c, count - c, offset + c));

    return c;

}

unsigned int fs_spawn(char *path)
{

    unsigned int service = fs_auth(path);

    if (service)
    {

        unsigned int id = fs_walk(service, 0, path);

        if (id)
        {

            unsigned int address = fs_map(service, id);

            return (address) ? call_spawn(address) : 0;

        }

    }

    return 0;

}

unsigned int fs_spawn_relative(char *path, char *parent)
{

    unsigned int service = fs_auth(parent);

    if (service)
    {

        unsigned int id = fs_walk(service, fs_walk(service, 0, parent), path);

        if (id)
        {

            unsigned int address = fs_map(service, id);

            return (address) ? call_spawn(address) : 0;

        }

    }

    return 0;

}

