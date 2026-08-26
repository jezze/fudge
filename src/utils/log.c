#include <fudge.h>
#include <abi.h>

static char *levels[5] = {
    "NULL",
    "CRIT",
    "ERRO",
    "WARN",
    "INFO"
};

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int log = channel_lookup(option_getstring("log-service"));
    struct message message;

    channel_send(0, log, EVENT_LINK, 0, 0);

    while (channel_poll(0, log, EVENT_LOGINFO, &message))
    {

        struct event_loginfo *loginfo = message_data(&message, 0);
        char *description = (char *)(loginfo + 1);
        unsigned int count = loginfo->count - sizeof (struct event_loginfo);

        if (option_getdecimal("level") >= loginfo->level)
            channel_send_fmt3(0, source, EVENT_DATA, "[%s] %w\n", levels[loginfo->level], description, &count);

    }

    channel_send(0, log, EVENT_UNLINK, 0, 0);

}

void init(void)
{

    option_add("log-service", "log");
    option_add("level", "4");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

