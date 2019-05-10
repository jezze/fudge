#include <fudge.h>
#include <abi.h>

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);

    file_open(file->descriptor);
    call_unload(file->descriptor);
    file_close(file->descriptor);

    return 0;

}

void init(void)
{

    event_initsignals();
    event_setsignal(EVENT_FILE, onfile);

}

void main(void)
{

    while (event_listen());

}

