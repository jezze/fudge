#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        struct message_header header;
        struct message_data data;
        char *path = "/data/help.txt";

        job_replyto(channel, id, EVENT_DATA, source);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(path), path);
        channel_place(channel, id, EVENT_MAIN, 0, 0);
        channel_poll(channel, id, EVENT_CLOSE, &header, &data);

    }

    channel_close(channel);

}

static void oninit(struct channel *channel)
{

    channel_setsignal(channel, EVENT_MAIN, onmain);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_listen(&channel, oninit);

}

