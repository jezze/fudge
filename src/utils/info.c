#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn("/bin/echo");

    if (id)
    {

        struct message_header header;
        struct message_data data;
        char *cores = "/system/info/cores";
        char *tasks = "/system/info/tasks";
        char *mailboxes = "/system/info/mailboxes";

        job_replyto(channel, id, EVENT_DATA, source);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(cores), cores);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(tasks), tasks);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(mailboxes), mailboxes);
        channel_place(channel, id, EVENT_MAIN, 0, 0);
        channel_pollsourceevent(channel, id, EVENT_CLOSE, &header, &data);

    }

    channel_close(channel);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

