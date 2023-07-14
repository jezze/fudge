#include <fudge.h>
#include <abi.h>

static unsigned int session;

static unsigned int getsession()
{

    return ++session;

}

static void sendlistrequest(unsigned int session, unsigned int id)
{

    struct event_listrequest listrequest;

    listrequest.session = session;
    listrequest.id = id;

    call_notify(FILE_G0, EVENT_LISTREQUEST, sizeof (struct event_listrequest), &listrequest);

}

static void sendreadrequest(unsigned int session, unsigned int id, unsigned int count, unsigned int offset)
{

    struct event_readrequest readrequest;

    readrequest.session = session;
    readrequest.id = id;
    readrequest.offset = offset;
    readrequest.count = count;

    call_notify(FILE_G0, EVENT_READREQUEST, sizeof (struct event_readrequest), &readrequest);

}

static void sendwalkrequest(unsigned int session, unsigned int parent, char *path)
{

    struct {struct event_walkrequest walkrequest; char path[64];} message;

    message.walkrequest.session = session;
    message.walkrequest.parent = parent;
    message.walkrequest.length = cstring_length(path);

    buffer_write(message.path, 64, path, message.walkrequest.length, 0);
    call_notify(FILE_G0, EVENT_WALKREQUEST, sizeof (struct event_walkrequest) + message.walkrequest.length, &message);

}

static unsigned int list(unsigned int id, struct record records[8])
{

    unsigned int session = getsession();
    struct message message;
    struct {struct event_listresponse listresponse; struct record records[8];} payload;

    sendlistrequest(session, id);

    while (channel_poll(EVENT_LISTRESPONSE, &message, &payload))
    {

        if (payload.listresponse.session == session)
        {

            buffer_write(records, sizeof (struct record) * 8, payload.records, sizeof (struct record) * payload.listresponse.nrecords, 0);

            return payload.listresponse.nrecords;

        }

    }

    return 0;

}

static unsigned int read(unsigned int id, unsigned int count, unsigned int offset, unsigned int ocount, void *obuffer)
{

    unsigned int session = getsession();
    struct message message;
    struct {struct event_readresponse readresponse; char data[64];} payload;

    if (!count)
        return 0;

    sendreadrequest(session, id, count, offset);

    while (channel_poll(EVENT_READRESPONSE, &message, &payload))
    {

        if (payload.readresponse.session == session)
        {

            buffer_write(obuffer, ocount, payload.data, payload.readresponse.count, 0);

            return payload.readresponse.count;

        }

    }

    return 0;

}

static unsigned int walk(unsigned int id, char *path)
{

    unsigned int session = getsession();
    struct message message;
    struct event_walkresponse walkresponse;

    sendwalkrequest(session, id, path);

    while (channel_poll(EVENT_WALKRESPONSE, &message, &walkresponse))
    {

        if (walkresponse.session == session)
            return walkresponse.id;

    }

    return 0;

}

static void test(void)
{

    unsigned int root = walk(0, "");
    unsigned int id = walk(root, "test.txt");

    if (root)
    {

        struct record records[8];
        unsigned int nrecords = list(root, records);
        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "(%u) %w\n", &record->id, record->name, &record->length);

        }

    }

    if (id)
    {

        char buffer[4096];
        unsigned int count = read(id, 100, 0, 4096, buffer);

        if (count)
            channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_G0, option_getstring("fs-service")))
        PANIC();

    call_link(FILE_G0, 8000);
    test();
    call_unlink(FILE_G0);

}

void init(void)
{

    option_add("fs-service", "system:service/fd0");
    channel_bind(EVENT_MAIN, onmain);

}

