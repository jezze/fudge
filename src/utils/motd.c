#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        struct message_header header;
        struct message_data data;
        char *path = "/data/motd.txt";

        job_replyto(channel, id, EVENT_DATA, source);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(path), path);
        channel_place(channel, id, EVENT_MAIN, 0, 0);
        channel_pollsourcetype(channel, id, EVENT_CLOSE, &header, &data);

    }

    channel_close(channel);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

