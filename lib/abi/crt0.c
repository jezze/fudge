#include <fudge.h>
#include <abi.h>

extern void init(void);

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthzero(key);

    if (!option_set(key, value))
        channel_warning("Unrecognized option");

}

static void onredirect(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_route(redirect->event, redirect->mode, redirect->id, source);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_close();

}

void main(void)
{

    channel_open();
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_REDIRECT, onredirect);
    channel_bind(EVENT_TERM, onterm);
    init();

    while (channel_process());

}

