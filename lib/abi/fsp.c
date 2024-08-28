#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fsp.h"

struct session
{

    unsigned int id;
    unsigned int ready;
    void *mdata;

};

static unsigned int sessioncount;
static struct session linksession;
static struct session listsession;
static struct session mapsession;
static struct session readsession;
static struct session statsession;
static struct session unlinksession;
static struct session walksession;
static struct session writesession;

static void session_init(struct session *session)
{

    session->id = ++sessioncount;
    session->ready = 0;

}

static void session_match(struct session *session, unsigned int id, void *mdata)
{

    if (session->id == id)
    {

        session->mdata = mdata;
        session->ready = 1;

    }

}

static unsigned int session_wait(struct session *session)
{

    while (channel_process())
    {

        if (session->ready)
            return 1;

    }

    return 0;

}

static void onlinkresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_linkresponse *header = mdata;

    session_match(&linksession, header->session, mdata);

}

static void onlistresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_listresponse *header = mdata;

    session_match(&listsession, header->session, mdata);

}

static void onmapresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mapresponse *header = mdata;

    session_match(&mapsession, header->session, mdata);

}

static void onreadresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_readresponse *header = mdata;

    session_match(&readsession, header->session, mdata);

}

static void onstatresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_statresponse *header = mdata;

    session_match(&statsession, header->session, mdata);

}

static void onunlinkresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_unlinkresponse *header = mdata;

    session_match(&unlinksession, header->session, mdata);

}

static void onwalkresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_walkresponse *header = mdata;

    session_match(&walksession, header->session, mdata);

}

static void onwriteresponse(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_writeresponse *header = mdata;

    session_match(&writesession, header->session, mdata);

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

    struct event_linkrequest request;

    session_init(&linksession);

    request.session = linksession.id;
    request.id = id;

    channel_send_buffer(target, EVENT_LINKREQUEST, sizeof (struct event_linkrequest), &request);

    if (session_wait(&linksession))
    {

        struct event_linkresponse *header = linksession.mdata;

        return header->id;

    }

    return 0;

}

unsigned int fsp_list(unsigned int target, unsigned int id, unsigned int offset, struct record *records, unsigned int nrecords)
{

    struct event_listrequest request;

    session_init(&listsession);

    request.session = listsession.id;
    request.id = id;
    request.offset = offset;
    request.nrecords = nrecords;

    channel_send_buffer(target, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &request);

    if (session_wait(&listsession))
    {

        struct event_listresponse *header = listsession.mdata;

        return buffer_write(records, sizeof (struct record) * nrecords, header + 1, sizeof (struct record) * header->nrecords, 0) / sizeof (struct record);

    }

    return 0;

}

unsigned int fsp_map(unsigned int target, unsigned int id)
{

    struct event_maprequest request;

    session_init(&mapsession);

    request.session = mapsession.id;
    request.id = id;

    channel_send_buffer(target, EVENT_MAPREQUEST, sizeof (struct event_maprequest), &request);

    if (session_wait(&mapsession))
    {

        struct event_mapresponse *header = mapsession.mdata;

        return header->address;

    }

    return 0;

}

unsigned int fsp_read(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_readrequest request;

    session_init(&readsession);

    request.session = readsession.id;
    request.id = id;
    request.offset = offset;
    request.count = count;

    channel_send_buffer(target, EVENT_READREQUEST, sizeof (struct event_readrequest), &request);

    if (session_wait(&readsession))
    {

        struct event_readresponse *header = readsession.mdata;

        return buffer_write(buffer, count, header + 1, header->count, 0);

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

    struct event_statrequest request;

    session_init(&statsession);

    request.session = statsession.id;
    request.id = id;

    channel_send_buffer(target, EVENT_STATREQUEST, sizeof (struct event_statrequest), &request);

    if (session_wait(&statsession))
    {

        struct event_statresponse *header = statsession.mdata;

        return buffer_write(record, sizeof (struct record), header + 1, sizeof (struct record) * header->nrecords, 0) / sizeof (struct record);

    }

    return 0;

}

unsigned int fsp_unlink(unsigned int target, unsigned int id)
{

    struct event_unlinkrequest request;

    session_init(&unlinksession);

    request.session = unlinksession.id;
    request.id = id;

    channel_send_buffer(target, EVENT_UNLINKREQUEST, sizeof (struct event_unlinkrequest), &request);

    if (session_wait(&unlinksession))
    {

        struct event_unlinkresponse *header = unlinksession.mdata;

        return header->id;

    }

    return 0;

}

unsigned int fsp_walk(unsigned int target, unsigned int parent, char *path)
{

    struct {struct event_walkrequest header; char path[64];} request;

    session_init(&walksession);

    request.header.session = walksession.id;
    request.header.parent = parent;
    request.header.length = buffer_write(request.path, 64, path, cstring_length(path), 0);

    channel_send_buffer(target, EVENT_WALKREQUEST, sizeof (struct event_walkrequest), &request);

    if (session_wait(&walksession))
    {

        struct event_walkresponse *header = walksession.mdata;

        return header->id;

    }

    return 0;

}

unsigned int fsp_write(unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct {struct event_writerequest header; char data[64];} request;

    session_init(&writesession);

    request.header.session = writesession.id;
    request.header.id = id;
    request.header.offset = offset;
    request.header.count = buffer_write(request.data, 64, buffer, count, 0);

    channel_send_buffer(target, EVENT_WRITEREQUEST, sizeof (struct event_writerequest), &request);

    if (session_wait(&writesession))
    {

        struct event_writeresponse *header = writesession.mdata;

        return buffer_write(buffer, count, header + 1, header->count, 0);

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
            return call_spawn(service, id, fsp_map(service, id));

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
            return call_spawn(service, id, fsp_map(service, id));

    }

    return 0;

}

void fsp_bind(void)
{

    channel_bind(EVENT_LINKRESPONSE, onlinkresponse);
    channel_bind(EVENT_LISTRESPONSE, onlistresponse);
    channel_bind(EVENT_MAPRESPONSE, onmapresponse);
    channel_bind(EVENT_READRESPONSE, onreadresponse);
    channel_bind(EVENT_STATRESPONSE, onstatresponse);
    channel_bind(EVENT_UNLINKRESPONSE, onunlinkresponse);
    channel_bind(EVENT_WALKRESPONSE, onwalkresponse);
    channel_bind(EVENT_WRITERESPONSE, onwriteresponse);

}
