#include <fudge.h>
#include <abi.h>

extern void init(void);

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = buffer_tindex(mdata, msize, '\0', 0);
    char *value = buffer_tindex(mdata, msize, '\0', 1);

    if (!option_set(key, value))
        channel_sendfmt0(CHANNEL_DEFAULT, EVENT_ERROR, "Unrecognized option\n");

}

static void onredirect(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_route(redirect->event, redirect->mode, redirect->id, source);

}

static void onstatus(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int i;

    for (i = 0; i < OPTION_MAX; i++)
    {

        struct option *option = option_get(i);

        if (option)
            channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "%s:%s\n", option->key, option->value);

    }

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
    channel_bind(EVENT_STATUS, onstatus);
    channel_bind(EVENT_TERM, onterm);
    init();

    while (channel_process());

}

