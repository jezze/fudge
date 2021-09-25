#include <fudge.h>
#include <abi.h>

extern void init(void);

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_close();

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_close();

}

static void onredirect(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_redirect(redirect, source);

}

void main(void)
{

    struct message_header header;
    struct message_data data;

    channel_init();
    channel_setcallback(EVENT_MAIN, onmain);
    channel_setcallback(EVENT_TERM, onterm);
    channel_setcallback(EVENT_REDIRECT, onredirect);
    init();

    while (channel_poll(&header, &data))
        channel_dispatch(&header, &data);

}

