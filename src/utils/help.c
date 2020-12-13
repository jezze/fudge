#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        struct message_header header;
        struct message_data data;

        channel_redirectsame(channel, id, EVENT_DATA);
        channel_redirectback(channel, id, EVENT_CLOSE);
        channel_placefor(channel, id, EVENT_FILE, message_putstringz(&data, "/data/help.txt", 0), &data);
        channel_placefor(channel, id, EVENT_MAIN, 0, 0);
        channel_pollsourceevent(channel, id, EVENT_CLOSE, &header, &data);

    }

    channel_close(channel);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

