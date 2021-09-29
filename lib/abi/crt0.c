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

    channel_redirect(redirect->event, redirect->mode, redirect->id, source);

}

void main(void)
{

    channel_open();
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_REDIRECT, onredirect);
    init();

    while (channel_process());

}

