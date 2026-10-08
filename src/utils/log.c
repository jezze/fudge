#include <fudge.h>
#include <abi.h>

static char *levels[5] = {
    "NULL",
    "CRIT",
    "ERRO",
    "WARN",
    "INFO"
};

static unsigned int output;

static void onloginfo(struct message *message)
{

    struct event_loginfo *loginfo = message->data;
    char *description = (char *)(loginfo + 1);
    unsigned int count = loginfo->count - sizeof (struct event_loginfo);

    if (option_getdecimal("level") >= loginfo->level)
        channel_send_fmt(0, output, EVENT_DATA, "[%s] %w\n", levels[loginfo->level], description, &count);

}

static void onmain(struct message *message)
{

    unsigned int log = channel_lookup(option_getstring("log-service"));

    if (log)
    {

        output = message->source;

        channel_send(0, log, EVENT_LINK, 0, 0);
        channel_hold(0);
        channel_send(0, log, EVENT_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("log-service", "log");
    option_add("level", "4");
    channel_bind(EVENT_LOGINFO, onloginfo);
    channel_bind(EVENT_MAIN, onmain);

}

