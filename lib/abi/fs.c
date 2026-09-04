#include <fudge.h>
#include <hash.h>
#include "call.h"
#include "channel.h"
#include "fs.h"

unsigned int fs_auth(char *path)
{

    unsigned int length = cstring_length(path);
    unsigned int offset = buffer_eachbyte(path, length, ':', 0);

    if (offset)
    {

        unsigned int namehash = djb_hash(offset - 1, path);

        return call_find(namehash, 0);

    }

    return 0;

}

unsigned int fs_list(unsigned int ichannel, unsigned int target, unsigned int id, unsigned int offset, struct record *records, unsigned int nrecords)
{

    struct event_listrequest request;
    unsigned char data[MESSAGE_SIZE];
    struct event_listresponse *response = (struct event_listresponse *)data;

    request.id = id;
    request.offset = offset;
    request.nrecords = nrecords;

    channel_send(ichannel, target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &request);
    channel_wait(ichannel, target, EVENT_LISTRESPONSE, MESSAGE_SIZE, data);

    return buffer_read(records, nrecords * sizeof (struct record), data + sizeof (struct event_listresponse), response->nrecords * sizeof (struct record), 0) / sizeof (struct record);

}

unsigned int fs_map(unsigned int ichannel, unsigned int target, unsigned int id)
{

    struct event_maprequest request;
    struct event_mapresponse response;

    request.id = id;

    channel_send(ichannel, target, EVENT_MAPREQUEST, sizeof (struct event_maprequest), &request);
    channel_wait(ichannel, target, EVENT_MAPRESPONSE, sizeof (struct event_mapresponse), &response);

    return response.address;

}

unsigned int fs_read(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_readrequest request;
    unsigned char data[MESSAGE_SIZE];
    struct event_readresponse *response = (struct event_readresponse *)data;

    request.id = id;
    request.offset = offset;
    request.count = count;

    channel_send(ichannel, target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request);
    channel_wait(ichannel, target, EVENT_READRESPONSE, MESSAGE_SIZE, data);

    return buffer_read(buffer, count, data + sizeof (struct event_readresponse), response->count, 0);

}

unsigned int fs_read_full(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *rbuffer = buffer;
    unsigned int rtotal = 0;
    unsigned int roffset;
    unsigned int rcount;

    for (roffset = offset; (rcount = fs_read(ichannel, target, id, rbuffer + rtotal, count - rtotal, roffset)); roffset += rcount)
        rtotal += rcount;

    return rtotal;

}

unsigned int fs_read_all(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += fs_read(ichannel, target, id, b + c, count - c, offset + c));

    return c;

}

unsigned int fs_stat(unsigned int ichannel, unsigned int target, unsigned int id, struct record *record)
{

    struct event_statrequest request;
    struct message message;

    request.id = id;

    channel_send(ichannel, target, EVENT_STATREQUEST, sizeof (struct event_statrequest), &request);
    channel_poll(ichannel, target, EVENT_STATRESPONSE, &message);

    return buffer_read(record, sizeof (struct record), message_data(&message, ichannel), message.length, 0) / sizeof (struct record);

}

unsigned int fs_walk(unsigned int ichannel, unsigned int target, unsigned int parent, char *path)
{

    struct event_walkrequest request;
    struct event_walkresponse response;
    unsigned int offset = buffer_eachbyte(path, cstring_length(path), ':', 0);
    char data[MESSAGE_SIZE];

    /* TODO: Handle this stuff better */
    if (offset)
        path += offset;

    request.parent = parent;
    request.length = buffer_write(data, MESSAGE_SIZE, path, cstring_length(path), sizeof (struct event_walkrequest));

    channel_send(ichannel, target, EVENT_WALKREQUEST, buffer_write(data, MESSAGE_SIZE, &request, sizeof (struct event_walkrequest), 0) + request.length, data);
    channel_wait(ichannel, target, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

    return response.id;

}

unsigned int fs_write(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_writerequest request;
    struct event_writeresponse response;
    char data[MESSAGE_SIZE];

    request.id = id;
    request.offset = offset;
    request.count = buffer_write(data, MESSAGE_SIZE, buffer, count, sizeof (struct event_writerequest));

    channel_send(ichannel, target, EVENT_WRITEREQUEST, buffer_write(data, MESSAGE_SIZE, &request, sizeof (struct event_writerequest), 0) + request.count, data);
    channel_wait(ichannel, target, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response);

    return response.count;

}

unsigned int fs_write_all(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += fs_write(ichannel, target, id, b + c, count - c, offset + c));

    return c;

}

unsigned int fs_spawn(unsigned int ichannel, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(ichannel, target, 0, path);

        if (id)
        {

            unsigned int address = fs_map(ichannel, target, id);

            return (address) ? call_spawn(address) : 0;

        }

    }

    return 0;

}

unsigned int fs_spawn_relative(unsigned int ichannel, char *path, char *parent)
{

    unsigned int target = fs_auth(parent);

    if (target)
    {

        unsigned int id = fs_walk(ichannel, target, fs_walk(ichannel, target, 0, parent), path);

        if (id)
        {

            unsigned int address = fs_map(ichannel, target, id);

            return (address) ? call_spawn(address) : 0;

        }

    }

    return 0;

}

