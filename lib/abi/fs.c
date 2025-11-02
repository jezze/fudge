#include <fudge.h>
#include <hash.h>
#include "call.h"
#include "channel.h"
#include "fs.h"

static unsigned int sessioncount;

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
    struct event_listresponse response;

    request.session = ++sessioncount;
    request.id = id;
    request.offset = offset;
    request.nrecords = nrecords;
    request.records = records;

    channel_send_buffer(ichannel, target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &request);

    while (channel_wait_buffer(ichannel, target, EVENT_LISTRESPONSE, sizeof (struct event_listresponse), &response))
    {

        if (response.session == request.session)
            return response.nrecords;

    }

    return 0;

}

unsigned int fs_map(unsigned int ichannel, unsigned int target, unsigned int id)
{

    struct event_maprequest request;
    struct event_mapresponse response;

    request.session = ++sessioncount;
    request.id = id;

    channel_send_buffer(ichannel, target, EVENT_MAPREQUEST, sizeof (struct event_maprequest), &request);

    while (channel_wait_buffer(ichannel, target, EVENT_MAPRESPONSE, sizeof (struct event_mapresponse), &response))
    {

        if (response.session == request.session)
            return response.address;

    }

    return 0;

}

unsigned int fs_read(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_readrequest request;
    struct event_readresponse response;

    request.session = ++sessioncount;
    request.id = id;
    request.offset = offset;
    request.count = count;
    request.buffer = buffer;

    channel_send_buffer(ichannel, target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request);

    while (channel_wait_buffer(ichannel, target, EVENT_READRESPONSE, sizeof (struct event_readresponse), &response))
    {

        if (response.session == request.session)
            return response.count;

    }

    return 0;

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
    struct event_statresponse response;

    request.session = ++sessioncount;
    request.id = id;
    request.record = record;

    channel_send_buffer(ichannel, target, EVENT_STATREQUEST, sizeof (struct event_statrequest), &request);

    while (channel_wait_buffer(ichannel, target, EVENT_STATRESPONSE, sizeof (struct event_statresponse), &response))
    {

        if (response.session == request.session)
            return response.nrecords;

    }

    return 0;

}

unsigned int fs_walk(unsigned int ichannel, unsigned int target, unsigned int parent, char *path)
{

    struct event_walkrequest request;
    struct event_walkresponse response;

    request.session = ++sessioncount;
    request.parent = parent;
    request.length = cstring_length(path);
    request.path = path;

    channel_send_buffer(ichannel, target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest), &request);

    while (channel_wait_buffer(ichannel, target, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response))
    {

        if (response.session == request.session)
            return response.id;

    }

    return 0;

}

unsigned int fs_write(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_writerequest request;
    struct event_writeresponse response;

    request.session = ++sessioncount;
    request.id = id;
    request.offset = offset;
    request.count = count;
    request.buffer = buffer;

    channel_send_buffer(ichannel, target, EVENT_WRITEREQUEST, sizeof (struct event_writerequest), &request);

    while (channel_wait_buffer(ichannel, target, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response))
    {

        if (response.session == request.session)
            return response.count;

    }

    return 0;

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

