#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fsp.h"

static unsigned int sessioncount;

static unsigned int getsession()
{

    return ++sessioncount;

}

static void sendlistrequest(unsigned int target, unsigned int session, unsigned int id, unsigned int cid)
{

    struct event_listrequest listrequest;

    listrequest.session = session;
    listrequest.id = id;
    listrequest.cid = cid;

    channel_send_buffer(target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &listrequest);

}

static void sendlistrequest2(unsigned int descriptor, unsigned int session, unsigned int id, unsigned int cid)
{

    struct event_listrequest listrequest;

    listrequest.session = session;
    listrequest.id = id;
    listrequest.cid = cid;

    call_notify(descriptor, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &listrequest);

}

static void sendlistresponse(unsigned int source, unsigned int session, unsigned int nrecords, struct record *records)
{

    struct {struct event_listresponse listresponse; struct record records[8];} message;

    message.listresponse.session = session;
    message.listresponse.nrecords = nrecords;

    buffer_write(message.records, sizeof (struct record) * 8, records, sizeof (struct record) * nrecords, 0);
    channel_send_buffer(source, EVENT_LISTRESPONSE, sizeof (struct event_listresponse) + sizeof (struct record) * message.listresponse.nrecords, &message);

}

static void sendreadrequest(unsigned int target, unsigned int session, unsigned int id, unsigned int count, unsigned int offset)
{

    struct event_readrequest readrequest;

    readrequest.session = session;
    readrequest.id = id;
    readrequest.offset = offset;
    readrequest.count = count;

    channel_send_buffer(target, EVENT_READREQUEST, sizeof (struct event_readrequest), &readrequest);

}

static void sendreadrequest2(unsigned int descriptor, unsigned int session, unsigned int id, unsigned int count, unsigned int offset)
{

    struct event_readrequest readrequest;

    readrequest.session = session;
    readrequest.id = id;
    readrequest.offset = offset;
    readrequest.count = count;

    call_notify(descriptor, EVENT_READREQUEST, sizeof (struct event_readrequest), &readrequest);

}

static void sendreadresponse(unsigned int source, unsigned int session, unsigned int count, void *buffer)
{

    struct {struct event_readresponse readresponse; char data[64];} message;

    message.readresponse.session = session;
    message.readresponse.count = count;

    buffer_write(message.data, 64, buffer, message.readresponse.count, 0);
    channel_send_buffer(source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + message.readresponse.count, &message);

}

static void sendwalkrequest(unsigned int target, unsigned int session, unsigned int parent, char *path)
{

    struct {struct event_walkrequest walkrequest; char path[64];} message;

    message.walkrequest.session = session;
    message.walkrequest.parent = parent;
    message.walkrequest.length = cstring_length(path);

    buffer_write(message.path, 64, path, message.walkrequest.length, 0);
    channel_send_buffer(target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest) + message.walkrequest.length, &message);

}

static void sendwalkrequest2(unsigned int descriptor, unsigned int session, unsigned int parent, char *path)
{

    struct {struct event_walkrequest walkrequest; char path[64];} message;

    message.walkrequest.session = session;
    message.walkrequest.parent = parent;
    message.walkrequest.length = cstring_length(path);

    buffer_write(message.path, 64, path, message.walkrequest.length, 0);
    call_notify(descriptor, EVENT_WALKREQUEST, sizeof (struct event_walkrequest) + message.walkrequest.length, &message);

}

static void sendwalkresponse(unsigned int source, unsigned int session, unsigned int id)
{

    struct event_walkresponse walkresponse;

    walkresponse.session = session;
    walkresponse.id = id;

    channel_send_buffer(source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &walkresponse);

}

unsigned int fsp_list(unsigned int target, unsigned int id, unsigned int cid, struct record records[8])
{

    unsigned int session = getsession();
    struct message message;
    struct {struct event_listresponse listresponse; struct record records[8];} payload;

    sendlistrequest(target, session, id, cid);

    while (channel_poll_any(EVENT_LISTRESPONSE, &message, &payload))
    {

        if (payload.listresponse.session == session)
        {

            buffer_write(records, sizeof (struct record) * 8, payload.records, sizeof (struct record) * payload.listresponse.nrecords, 0);

            return payload.listresponse.nrecords;

        }

    }

    return 0;

}

unsigned int fsp_list2(unsigned int descriptor, unsigned int id, unsigned int cid, struct record records[8])
{

    unsigned int session = getsession();
    struct message message;
    struct {struct event_listresponse listresponse; struct record records[8];} payload;

    sendlistrequest2(descriptor, session, id, cid);

    while (channel_poll_any(EVENT_LISTRESPONSE, &message, &payload))
    {

        if (payload.listresponse.session == session)
        {

            buffer_write(records, sizeof (struct record) * 8, payload.records, sizeof (struct record) * payload.listresponse.nrecords, 0);

            return payload.listresponse.nrecords;

        }

    }

    return 0;

}

void fsp_listresponse(unsigned int source, unsigned int session, unsigned int nrecords, struct record *records)
{

    sendlistresponse(source, session, nrecords, records);

}

unsigned int fsp_read(unsigned int target, unsigned int id, unsigned int count, unsigned int offset, unsigned int ocount, void *obuffer)
{

    unsigned int session = getsession();
    struct message message;
    struct {struct event_readresponse readresponse; char data[64];} payload;

    if (!count)
        return 0;

    sendreadrequest(target, session, id, count, offset);

    while (channel_poll_any(EVENT_READRESPONSE, &message, &payload))
    {

        if (payload.readresponse.session == session)
        {

            buffer_write(obuffer, ocount, payload.data, payload.readresponse.count, 0);

            return payload.readresponse.count;

        }

    }

    return 0;

}

unsigned int fsp_read2(unsigned int descriptor, unsigned int id, unsigned int count, unsigned int offset, unsigned int ocount, void *obuffer)
{

    unsigned int session = getsession();
    struct message message;
    struct {struct event_readresponse readresponse; char data[64];} payload;

    if (!count)
        return 0;

    sendreadrequest2(descriptor, session, id, count, offset);

    while (channel_poll_any(EVENT_READRESPONSE, &message, &payload))
    {

        if (payload.readresponse.session == session)
        {

            buffer_write(obuffer, ocount, payload.data, payload.readresponse.count, 0);

            return payload.readresponse.count;

        }

    }

    return 0;

}

void fsp_readresponse(unsigned int source, unsigned int session, unsigned int count, void *buffer)
{

    sendreadresponse(source, session, count, buffer);

}

unsigned int fsp_walk(unsigned int target, unsigned int id, char *path)
{

    unsigned int session = getsession();
    struct message message;
    struct event_walkresponse walkresponse;

    sendwalkrequest(target, session, id, path);

    while (channel_poll_any(EVENT_WALKRESPONSE, &message, &walkresponse))
    {

        if (walkresponse.session == session)
            return walkresponse.id;

    }

    return 0;

}

unsigned int fsp_walk2(unsigned int descriptor, unsigned int id, char *path)
{

    unsigned int session = getsession();
    struct message message;
    struct event_walkresponse walkresponse;

    sendwalkrequest2(descriptor, session, id, path);

    while (channel_poll_any(EVENT_WALKRESPONSE, &message, &walkresponse))
    {

        if (walkresponse.session == session)
            return walkresponse.id;

    }

    return 0;

}

void fsp_walkresponse(unsigned int source, unsigned int session, unsigned int id)
{

    sendwalkresponse(source, session, id);

}

