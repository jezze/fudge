#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fsp.h"

static unsigned int sessioncount;

static unsigned int getsession()
{

    return ++sessioncount;

}

unsigned int fsp_auth(char *path)
{

    /* need to make syscall for this */

    if (cstring_length(path) >= 7 && buffer_match(path, "initrd:", 7))
        return 666;

    if (cstring_length(path) >= 7 && buffer_match(path, "system:", 7))
        return 667;

    return 0;

}

unsigned int fsp_list(unsigned int target, unsigned int id, unsigned int offset, struct record records[8])
{

    unsigned int session = getsession();
    struct event_listrequest request;

    request.session = session;
    request.id = id;
    request.offset = offset;

    if (channel_send_buffer(target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &request))
    {

        struct message message;
        char data[MESSAGE_SIZE];

        while (channel_poll_any(EVENT_LISTRESPONSE, &message, MESSAGE_SIZE, data))
        {

            struct {struct event_listresponse header; struct record records[8];} *response = (void *)data;

            if (response->header.session == session)
            {

                buffer_write(records, sizeof (struct record) * 8, response->records, sizeof (struct record) * response->header.nrecords, 0);

                return response->header.nrecords;

            }

        }

    }

    return 0;

}

unsigned int fsp_listresponse(unsigned int source, unsigned int session, unsigned int nrecords, struct record *records)
{

    struct {struct event_listresponse header; struct record records[8];} response;

    response.header.session = session;
    response.header.nrecords = nrecords;

    buffer_write(response.records, sizeof (struct record) * 8, records, sizeof (struct record) * nrecords, 0);

    return channel_send_buffer(source, EVENT_LISTRESPONSE, sizeof (struct event_listresponse) + sizeof (struct record) * response.header.nrecords, &response);

}

unsigned int fsp_read(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned int session = getsession();
    struct event_readrequest request;

    request.session = session;
    request.id = id;
    request.offset = offset;
    request.count = count;

    if (channel_send_buffer(target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request))
    {

        struct message message;
        char data[MESSAGE_SIZE];

        while (channel_poll_any(EVENT_READRESPONSE, &message, MESSAGE_SIZE, data))
        {

            struct event_readresponse *response = (struct event_readresponse *)data;

            if (response->session == session)
                return buffer_write(buffer, count, response + 1, response->count, 0);

        }

    }

    return 0;

}

unsigned int fsp_read_full(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *rbuffer = buffer;
    unsigned int rtotal = 0;
    unsigned int roffset;
    unsigned int rcount;

    for (roffset = offset; (rcount = fsp_read(target, id, rbuffer + roffset, count - roffset, roffset)); roffset += rcount)
        rtotal += rcount;

    return rtotal;

}

unsigned int fsp_read_all(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += fsp_read(target, id, b + c, count - c, offset + c));

    return c;

}

unsigned int fsp_walk(unsigned int target, unsigned int parent, char *path)
{

    unsigned int session = getsession();
    struct {struct event_walkrequest header; char path[64];} request;

    request.header.session = session;
    request.header.parent = parent;
    request.header.length = buffer_write(request.path, 64, path, cstring_length(path), 0);

    if (channel_send_buffer(target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest) + request.header.length, &request))
    {

        struct message message;
        char data[MESSAGE_SIZE];

        while (channel_poll_any(EVENT_WALKRESPONSE, &message, MESSAGE_SIZE, data))
        {

            struct {struct event_walkresponse header;} *response = (void *)data;

            if (response->header.session == session)
                return response->header.id;

        }

    }

    return 0;

}

unsigned int fsp_write(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned int session = getsession();
    struct {struct event_writerequest header; char data[64];} request;

    request.header.session = session;
    request.header.id = id;
    request.header.offset = offset;
    request.header.count = buffer_write(request.data, 64, buffer, count, 0);

    if (channel_send_buffer(target, EVENT_WRITEREQUEST, sizeof (struct event_writerequest) + request.header.count, &request))
    {

        struct message message;
        char data[MESSAGE_SIZE];

        while (channel_poll_any(EVENT_WRITERESPONSE, &message, MESSAGE_SIZE, data))
        {

            struct event_writeresponse *response = (struct event_writeresponse *)data;

            if (response->session == session)
                return response->count;

        }

    }

    return 0;

}

unsigned int fsp_write_all(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += fsp_write(target, id, b + c, count - c, offset + c));

    return c;

}

unsigned int fsp_link(unsigned int target, unsigned int id)
{

    unsigned int session = getsession();
    struct event_linkrequest request;

    request.session = session;
    request.id = id;

    if (channel_send_buffer(target, EVENT_LINKREQUEST, sizeof (struct event_linkrequest), &request))
    {

        struct message message;
        char data[MESSAGE_SIZE];

        if (channel_poll_any(EVENT_LINKRESPONSE, &message, MESSAGE_SIZE, data))
            return 1;

    }

    return 0;

}

unsigned int fsp_unlink(unsigned int target, unsigned int id)
{

    unsigned int session = getsession();
    struct event_unlinkrequest request;

    request.session = session;
    request.id = id;

    if (channel_send_buffer(target, EVENT_UNLINKREQUEST, sizeof (struct event_unlinkrequest), &request))
    {

        struct message message;
        char data[MESSAGE_SIZE];

        if (channel_poll_any(EVENT_UNLINKRESPONSE, &message, MESSAGE_SIZE, data))
            return 1;

    }

    return 0;

}

unsigned int fsp_spawn(char *path)
{

    unsigned int service = fsp_auth(path);

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, path);

        if (id)
            return call_spawn(service, id);

    }

    return 0;

}

unsigned int fsp_spawn_relative(char *path, char *parent)
{

    unsigned int service = fsp_auth(parent);

    if (service)
    {

        unsigned int id = fsp_walk(service, fsp_walk(service, 0, parent), path);

        if (id)
            return call_spawn(service, id);

    }

    return 0;

}

