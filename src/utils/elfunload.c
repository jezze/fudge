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

static unsigned int onstop(struct event_channel *channel)
{

    return 1;

}

static unsigned int onkill(struct event_channel *channel)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    while (!status)
    {

        switch (event_pick(&channel))
        {

        case EVENT_FILE:
            status = onfile(&channel);

            break;

        case EVENT_STOP:
            status = onstop(&channel);

            break;

        case EVENT_KILL:
            status = onkill(&channel);

            break;

        }

    }

}

