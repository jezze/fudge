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

    unsigned char data[MESSAGE_SIZE];
    struct event_readrequest request;
    struct event_readresponse *response = (struct event_readresponse *)data;

    request.id = id;
    request.offset = offset;
    request.count = count;

    channel_send(ichannel, target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request);
    channel_wait(ichannel, target, EVENT_READRESPONSE, MESSAGE_SIZE, data);
    buffer_copy(buffer, response + 1, response->count);

    return response->count;

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

    request.id = id;

    channel_send(ichannel, target, EVENT_STATREQUEST, sizeof (struct event_statrequest), &request);
    channel_wait(ichannel, target, EVENT_STATRESPONSE, sizeof (struct record), record);

    return 1;

}

unsigned int fs_walk(unsigned int ichannel, unsigned int target, unsigned int parent, char *path)
{

    char data[MESSAGE_SIZE];
    struct event_walkrequest *request = (struct event_walkrequest *)data;
    struct event_walkresponse response;

    /* TODO: Handle this stuff better */
    path += buffer_eachbyte(path, cstring_length(path), ':', 0);

    request->parent = parent;
    request->length = buffer_write(data, MESSAGE_SIZE, path, cstring_length(path), sizeof (struct event_walkrequest));

    channel_send(ichannel, target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest) + request->length, data);
    channel_wait(ichannel, target, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

    return response.id;

}

unsigned int fs_write(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    char data[MESSAGE_SIZE];
    struct event_writerequest *request = (struct event_writerequest *)data;
    struct event_writeresponse response;

    request->id = id;
    request->offset = offset;
    request->count = buffer_write(data, MESSAGE_SIZE, buffer, count, sizeof (struct event_writerequest));

    channel_send(ichannel, target, EVENT_WRITEREQUEST, sizeof (struct event_writerequest) + request->count, data);
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

