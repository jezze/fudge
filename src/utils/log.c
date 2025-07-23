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

    char data[MESSAGE_SIZE];
    struct message message;

    channel_send(0, option_getdecimal("log-service"), EVENT_LINK);

    while (channel_poll(0, option_getdecimal("log-service"), EVENT_LOGINFO, &message, MESSAGE_SIZE, data))
    {

        struct event_loginfo *loginfo = (void *)data;
        char *description = (char *)(loginfo + 1);
        unsigned int count = loginfo->count - sizeof (struct event_loginfo);

        if (option_getdecimal("level") >= loginfo->level)
            channel_send_fmt3(0, source, EVENT_DATA, "[%s] %w\n", levels[loginfo->level], description, &count);

    }

    channel_send(0, option_getdecimal("log-service"), EVENT_UNLINK);

}

void init(void)
{

    option_add("log-service", "0");
    option_add("level", "4");
    channel_bind(EVENT_MAIN, onmain);

}

