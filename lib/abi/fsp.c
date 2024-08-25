#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fsp.h"

struct session
{

    unsigned int session;
    unsigned int ready;
    void *mdata;

};

static unsigned int sessioncount;
static struct session linksession;
static struct session listsession;
static struct session readsession;
static struct session statsession;
static struct session unlinksession;
static struct session walksession;
static struct session writesession;

static unsigned int getsession()
{

    return ++sessioncount;

}

static void onlinkresponse(unsigned int source, void *mdata, unsigned int msize)
{

    linksession.mdata = mdata;
    linksession.ready = 1;

}

static void onlistresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct {struct event_listresponse header;} *response = mdata;

    if (response->header.session == listsession.session)
    {

        listsession.mdata = mdata;
        listsession.ready = 1;

    }

}

static void onreadresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct {struct event_readresponse header;} *response = mdata;

    if (response->header.session == readsession.session)
    {

        readsession.mdata = mdata;
        readsession.ready = 1;

    }

}

static void onstatresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct {struct event_statresponse header;} *response = mdata;

    if (response->header.session == statsession.session)
    {

        statsession.mdata = mdata;
        statsession.ready = 1;

    }

}

static void onunlinkresponse(unsigned int source, void *mdata, unsigned int msize)
{

    unlinksession.mdata = mdata;
    unlinksession.ready = 1;

}

static void onwalkresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct {struct event_walkresponse header;} *response = mdata;

    if (response->header.session == walksession.session)
    {

        walksession.mdata = mdata;
        walksession.ready = 1;

    }

}

static void onwriteresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct {struct event_writeresponse header;} *response = mdata;

    if (response->header.session == writesession.session)
    {

        writesession.mdata = mdata;
        writesession.ready = 1;

    }

}

unsigned int fsp_auth(char *path)
{

    /* need to make syscall for this */

    if (cstring_length(path) >= 7 && buffer_match(path, "initrd:", 7))
        return 500;

    if (cstring_length(path) >= 7 && buffer_match(path, "system:", 7))
        return 501;

    return 0;

}

unsigned int fsp_link(unsigned int target, unsigned int id)
{

    unsigned int session = getsession();
    struct event_linkrequest request;

    request.session = session;
    request.id = id;

    linksession.ready = 0;
    linksession.session = session;

    channel_send_buffer(target, EVENT_LINKREQUEST, sizeof (struct event_linkrequest), &request);

    while (channel_process())
    {

        if (linksession.ready)
            return 1;

    }

    return 0;

}

unsigned int fsp_list(unsigned int target, unsigned int id, unsigned int offset, struct record *records, unsigned int nrecords)
{

    unsigned int session = getsession();
    struct event_listrequest request;

    request.session = session;
    request.id = id;
    request.offset = offset;
    request.nrecords = nrecords;

    listsession.ready = 0;
    listsession.session = session;

    channel_send_buffer(target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &request);

    while (channel_process())
    {

        if (listsession.ready)
        {

            struct event_listresponse *header = listsession.mdata;

            buffer_write(records, sizeof (struct record) * nrecords, header + 1, sizeof (struct record) * header->nrecords, 0);

            return header->nrecords;

        }

    }

    return 0;

}

unsigned int fsp_read(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned int session = getsession();
    struct event_readrequest request;

    request.session = session;
    request.id = id;
    request.offset = offset;
    request.count = count;

    readsession.ready = 0;
    readsession.session = session;

    channel_send_buffer(target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request);

    while (channel_process())
    {

        if (readsession.ready)
        {

            struct event_readresponse *header = readsession.mdata;

            return buffer_write(buffer, count, header + 1, header->count, 0);

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

    for (roffset = offset; (rcount = fsp_read(target, id, rbuffer + rtotal, count - rtotal, roffset)); roffset += rcount)
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

unsigned int fsp_stat(unsigned int target, unsigned int id, struct record *record)
{

    unsigned int session = getsession();
    struct event_statrequest request;

    request.session = session;
    request.id = id;

    statsession.ready = 0;
    statsession.session = session;

    channel_send_buffer(target, EVENT_STATREQUEST, sizeof (struct event_statrequest), &request);

    while (channel_process())
    {

        if (statsession.ready)
        {

            struct event_statresponse *header = statsession.mdata;

            buffer_write(record, sizeof (struct record), header + 1, sizeof (struct record) * header->nrecords, 0);

            return header->nrecords;

        }

    }

    return 0;

}

unsigned int fsp_unlink(unsigned int target, unsigned int id)
{

    unsigned int session = getsession();
    struct event_unlinkrequest request;

    request.session = session;
    request.id = id;

    unlinksession.ready = 0;
    unlinksession.session = session;

    channel_send_buffer(target, EVENT_UNLINKREQUEST, sizeof (struct event_unlinkrequest), &request);

    while (channel_process())
    {

        if (unlinksession.ready)
            return 1;

    }

    return 0;

}

unsigned int fsp_walk(unsigned int target, unsigned int parent, char *path)
{

    unsigned int session = getsession();
    struct {struct event_walkrequest header; char path[64];} request;

    request.header.session = session;
    request.header.parent = parent;
    request.header.length = buffer_write(request.path, 64, path, cstring_length(path), 0);

    walksession.ready = 0;
    walksession.session = session;

    channel_send_buffer(target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest), &request);

    while (channel_process())
    {

        if (walksession.ready)
        {

            struct event_walkresponse *header = walksession.mdata;

            return header->id;

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

    writesession.ready = 0;
    writesession.session = session;

    channel_send_buffer(target, EVENT_WRITEREQUEST, sizeof (struct event_writerequest), &request);

    while (channel_process())
    {

        if (writesession.ready)
        {

            struct event_writeresponse *header = writesession.mdata;

            return buffer_write(buffer, count, header + 1, header->count, 0);

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

void fsp_bind(void)
{

    channel_bind(EVENT_LINKRESPONSE, onlinkresponse);
    channel_bind(EVENT_LISTRESPONSE, onlistresponse);
    channel_bind(EVENT_READRESPONSE, onreadresponse);
    channel_bind(EVENT_STATRESPONSE, onstatresponse);
    channel_bind(EVENT_UNLINKRESPONSE, onunlinkresponse);
    channel_bind(EVENT_WALKRESPONSE, onwalkresponse);
    channel_bind(EVENT_WRITERESPONSE, onwriteresponse);

}
