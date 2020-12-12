#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        struct message_header header;
        struct message_data data;

        job_replyto(channel, id, EVENT_DATA, (channel->callbacks[EVENT_DATA].redirect) ? channel->callbacks[EVENT_DATA].redirect : source);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place2(channel, id, EVENT_FILE, message_putstringz(&data, "/data/help.txt", 0), &data);
        channel_place2(channel, id, EVENT_MAIN, 0, 0);
        channel_pollsourceevent(channel, id, EVENT_CLOSE, &header, &data);

    }

    channel_close(channel, source);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

