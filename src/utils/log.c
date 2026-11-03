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

    struct message_loginfo *loginfo = message_validate(MESSAGE_TYPE_LOGINFO, message->data, message->length);

    if (loginfo && option_getdecimal("level") >= loginfo->level)
        channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "[%s] %w\n", levels[loginfo->level], loginfo + 1, &loginfo->count);

}

static void onmain(struct message *message)
{

    unsigned int log = channel_lookup(option_getstring("log-service"));

    if (log)
    {

        output = message->source;

        channel_send(0, log, MESSAGE_TYPE_LINK, 0, 0);
        channel_hold(0);
        channel_send(0, log, MESSAGE_TYPE_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("log-service", "log");
    option_add("level", "4");
    channel_bind(MESSAGE_TYPE_LOGINFO, onloginfo);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

