#include <fudge.h>
#include <abi.h>

extern void init(void);

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = buffer_tindex(mdata, msize, '\0', 0);
    char *value = buffer_tindex(mdata, msize, '\0', 1);

    if (!option_set(key, value))
        channel_send_fmt0(CHANNEL_DEFAULT, EVENT_ERROR, "Unrecognized option\n");

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
            channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "%s:%s\n", option->key, option->value);

    }

}

void panic(char *file, unsigned int line)
{

    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_ERROR, "Process panic! File %s on line %u\n", file, &line);
    channel_close();
    call_despawn();

}

void main(void)
{

    channel_open();
    channel_autoclose(EVENT_MAIN, 1);
    channel_autoclose(EVENT_TERM, 1);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_REDIRECT, onredirect);
    channel_bind(EVENT_STATUS, onstatus);
    init();

    while (channel_process());

}

