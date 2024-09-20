#include <fudge.h>
#include <abi.h>

static unsigned int wmservice;

static void onqueryrequest(unsigned int source, void *mdata, unsigned int msize)
{

    char *query = mdata;

    if (buffer_match(query, "get", 3))
    {

        if (buffer_match(query + 4, "wm-service", 10))
            channel_send_fmt1(source, EVENT_QUERYRESPONSE, "%u\\0", &wmservice);

    }

    if (buffer_match(query, "set", 3))
    {

        if (buffer_match(query + 4, "wm-service", 10))
            wmservice = source;

    }

}

void init(void)
{

    channel_bind(EVENT_QUERYREQUEST, onqueryrequest);

}

