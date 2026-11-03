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

    struct message_loginfo *loginfo = message_validate(MESSAGE_LOGINFO, message->data, message->length);

    if (loginfo && option_getdecimal("level") >= loginfo->level)
    {

        char *description = (char *)(loginfo + 1);
        unsigned int count = loginfo->count - sizeof (struct message_loginfo);

        channel_send_fmt(0, output, MESSAGE_DATA, "[%s] %w\n", levels[loginfo->level], description, &count);

    }

}

static void onmain(struct message *message)
{

    unsigned int log = channel_lookup(option_getstring("log-service"));

    if (log)
    {

        output = message->source;

        channel_send(0, log, MESSAGE_LINK, 0, 0);
        channel_hold(0);
        channel_send(0, log, MESSAGE_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("log-service", "log");
    option_add("level", "4");
    channel_bind(MESSAGE_LOGINFO, onloginfo);
    channel_bind(MESSAGE_MAIN, onmain);

}

