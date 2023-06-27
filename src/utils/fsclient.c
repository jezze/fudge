#include <fudge.h>
#include <abi.h>

static void readfile(unsigned int id, unsigned int count, unsigned int offset)
{

    struct event_read read;

    read.id = id;
    read.offset = offset;
    read.count = count;

    call_notify(FILE_G0, EVENT_READ, sizeof (struct event_read), &read);

    while (channel_process());

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_G0, option_getstring("fs-service")))
        PANIC();

    call_link(FILE_G0, 8000);
    readfile(12, 100, 0);
    call_unlink(FILE_G0);

}

void init(void)
{

    option_add("fs-service", "system:service/fd0");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

