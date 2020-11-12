#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        struct message_header header;
        char *path = "/data/motd.txt";
        char data[FUDGE_MSIZE];

        job_replyto(channel, id, EVENT_DATA, (channel->signals[EVENT_DATA].redirect) ? (channel->signals[EVENT_DATA].redirect): source);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place(channel, id, EVENT_FILE, ascii_length(path) + 1, path);
        channel_place(channel, id, EVENT_MAIN, 0, 0);
        channel_poll(channel, id, EVENT_CLOSE, &header, data);

    }

    channel_close(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_listen(&channel, 0, 0);

}

