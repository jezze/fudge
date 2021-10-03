#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    p9p_walk(&message, "build/data/help.txt");
    file_notify(FILE_G0, EVENT_P9P, message_datasize(&message.header), message.data.buffer);
    channel_pollevent(EVENT_P9P, &message);
    p9p_read(&message);
    file_notify(FILE_G0, EVENT_P9P, message_datasize(&message.header), message.data.buffer);
    channel_pollevent(EVENT_P9P, &message);
    channel_sendbuffer(EVENT_DATA, message_datasize(&message.header) - sizeof (struct event_p9p), message.data.buffer + sizeof (struct event_p9p));
    channel_close();

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/fd0"))
        return;

    channel_bind(EVENT_MAIN, onmain);

}

