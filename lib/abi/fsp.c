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

    if (cstring_match(path, "initrd"))
        return 666;

    if (cstring_length(path) > 7 && buffer_match(path, "initrd:", 7))
        return 666;

    if (cstring_match(path, "system"))
        return 667;

    if (cstring_length(path) > 7 && buffer_match(path, "system:", 7))
        return 667;


    return 0;

}

unsigned int fsp_list(unsigned int target, unsigned int id, unsigned int cid, struct record records[8])
{

    unsigned int session = getsession();
    struct {struct event_listrequest header;} request;

    request.header.session = session;
    request.header.id = id;
    request.header.cid = cid;

    if (channel_send_buffer(target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &request))
    {

        struct message message;
        struct {struct event_listresponse listresponse; struct record records[8];} payload;

        while (channel_poll_any(EVENT_LISTRESPONSE, &message, &payload))
        {

            if (payload.listresponse.session == session)
            {

                buffer_write(records, sizeof (struct record) * 8, payload.records, sizeof (struct record) * payload.listresponse.nrecords, 0);

                return payload.listresponse.nrecords;

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

unsigned int fsp_read(unsigned int target, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int session = getsession();
    struct {struct event_readrequest header;} request;

    if (!count)
        return 0;

    request.header.session = session;
    request.header.id = id;
    request.header.offset = offset;
    request.header.count = count;

    if (channel_send_buffer(target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request))
    {

        struct message message;
        struct {struct event_readresponse readresponse; char data[64];} payload;

        while (channel_poll_any(EVENT_READRESPONSE, &message, &payload))
        {

            if (payload.readresponse.session == session)
            {

                buffer_write(buffer, count, payload.data, payload.readresponse.count, 0);

                return payload.readresponse.count;

            }

        }

    }

    return 0;

}

unsigned int fsp_readresponse(unsigned int source, unsigned int session, unsigned int count, void *buffer)
{

    struct {struct event_readresponse header; char data[64];} response;

    response.header.session = session;
    response.header.count = count;

    buffer_write(response.data, 64, buffer, response.header.count, 0);

    return channel_send_buffer(source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response.header.count, &response);

}

unsigned int fsp_walk(unsigned int target, unsigned int parent, char *path)
{

    unsigned int session = getsession();
    struct {struct event_walkrequest header; char path[64];} request;
    unsigned int offset = buffer_firstbyte(path, cstring_length(path), ':');

    if (offset)
        path += offset;

    request.header.session = session;
    request.header.parent = parent;
    request.header.length = cstring_length(path);

    buffer_write(request.path, 64, path, request.header.length, 0);

    if (channel_send_buffer(target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest) + request.header.length, &request))
    {

        struct message message;
        struct event_walkresponse walkresponse;

        while (channel_poll_any(EVENT_WALKRESPONSE, &message, &walkresponse))
        {

            if (walkresponse.session == session)
                return walkresponse.id;

        }

    }

    return 0;

}

unsigned int fsp_walkresponse(unsigned int source, unsigned int session, unsigned int id)
{

    struct {struct event_walkresponse header;} response;

    response.header.session = session;
    response.header.id = id;

    return channel_send_buffer(source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

