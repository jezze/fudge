#include <fudge.h>
#include <abi.h>

static void onlistrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_listrequest *listrequest = mdata;
    struct record records[8];

    call_set(FILE_L0, listrequest->id);
    fsp_listresponse(source, listrequest->session, call_list(FILE_L0, listrequest->offset, 8, records), records);

}

static void onreadrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_readrequest *readrequest = mdata;
    char buffer[MESSAGE_SIZE];

    call_set(FILE_L0, readrequest->id);
    fsp_readresponse(source, readrequest->session, call_read(FILE_L0, buffer, readrequest->count < 64 ? readrequest->count : 64, readrequest->offset), buffer);

}

static void onwalkrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_walkrequest *walkrequest = mdata;

    if (!walkrequest->parent)
        walkrequest->parent = call_walk_absolute(FILE_L0, "/");

    if (walkrequest->length)
    {

        char buffer[200];

        buffer_write(buffer, 200, walkrequest + 1, walkrequest->length, 0);

        buffer[walkrequest->length] = '\0';

        call_set(FILE_L0, walkrequest->parent);
        fsp_walkresponse(source, walkrequest->session, call_walk_relative(FILE_L1, FILE_L0, buffer));

    }

    else
    {

        fsp_walkresponse(source, walkrequest->session, walkrequest->parent);

    }

}

static void onwriterequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_writerequest *writerequest = mdata;
    char buffer[MESSAGE_SIZE];

    call_set(FILE_L0, writerequest->id);
    fsp_writeresponse(source, writerequest->session, call_write(writerequest->id, buffer, writerequest->count < 64 ? writerequest->count : 64, writerequest->offset), buffer);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    call_announce(option_getdecimal("listen"));

    while (channel_process());

}

void init(void)
{

    option_add("listen", "90");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_LISTREQUEST, onlistrequest);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

}

