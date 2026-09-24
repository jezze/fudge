#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct event_blockinfo blockinfo;

/*
static unsigned int sendblockreadrequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    if (target)
    {

        struct event_blockrequest request;
        struct event_blockresponse response;

        request.offset = option_getdecimal("partoffset") + sector * blocksize;
        request.count = count;

        channel_send(0, target, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(0, target, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        return response.count;

    }

    return 0;

}

static unsigned int sendblockwriterequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    if (target)
    {

        struct event_blockrequest request;
        struct event_blockresponse response;

        request.offset = option_getdecimal("partoffset") + sector * blocksize;
        request.count = count;

        channel_send(0, target, EVENT_BLOCKWRITEREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(0, target, EVENT_BLOCKWRITERESPONSE, sizeof (struct event_blockresponse), &response);

        return response.count;

    }

    return 0;

}
*/

static unsigned int stat(unsigned int id, struct record *record)
{

    buffer_clear(&record, sizeof (struct record));

    return sizeof (struct record);

}

static void oncreaterequest(struct message *message)
{

    /*struct event_createrequest *request = message->data;*/
    struct event_createresponse response;

    response.id = 0;

    channel_send(0, message->source, EVENT_CREATERESPONSE, sizeof (struct event_createresponse), &response);

}

static void onreadrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    /*struct event_readrequest *request = message->data;*/
    struct event_readresponse *response = (struct event_readresponse *)data;

    response->count = 0;

    channel_send(0, message->source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, data);

}

static void onremoverequest(struct message *message)
{

    /*struct event_removerequest *request = message->data;*/
    struct event_removeresponse response;

    response.status = 0;

    channel_send(0, message->source, EVENT_REMOVERESPONSE, sizeof (struct event_removeresponse), &response);

}

static void onstatrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct event_statrequest *request = message->data;
    struct event_statresponse *response = (struct event_statresponse *)data;

    response->count = stat(request->id, (struct record *)(response + 1));

    channel_send(0, message->source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, data);

}

static void onwalkrequest(struct message *message)
{

    /*struct event_walkrequest *request = message->data;*/
    struct event_walkresponse response;

    response.id = 0;

    channel_send(0, message->source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static void onwriterequest(struct message *message)
{

    /*struct event_writerequest *request = message->data;*/
    struct event_writeresponse response;

    response.count = 0;

    channel_send(0, message->source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response);

}

static void onmain(struct message *message)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));

    if (block)
    {

        channel_send(0, block, EVENT_INFO, 0, 0);
        channel_wait(0, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

    }

}

void init(void)
{

    option_add("service", "fat");
    option_add("block-service", "block");
    option_add("partoffset", "0");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_CREATEREQUEST, oncreaterequest);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_REMOVEREQUEST, onremoverequest);
    channel_bind(EVENT_STATREQUEST, onstatrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

    while (channel_process(0));

}

