#include <fudge.h>
#include <abi.h>

#define LEVELS                          5

static char *levels[LEVELS] = {
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

    if (loginfo && loginfo->level < LEVELS && option_getdecimal("level") >= loginfo->level)
    {

        char *text = (char *)(loginfo + 1);
        unsigned int count = loginfo->count;

        while (count && text[count - 1] == '\n')
            count--;

        channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "[%s] %w\n", levels[loginfo->level], text, &count);

    }

}

static void onmain(struct message *message)
{

    unsigned int log = channel_lookup(option_getstring("log-service"));

    if (log)
    {

        output = (cstring_length(option_getstring("output-service"))) ? channel_lookup(option_getstring("output-service")) : message->source;

        channel_send(0, log, MESSAGE_TYPE_LINK, 0, 0);
        channel_hold(0);
        channel_send(0, log, MESSAGE_TYPE_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("log-service", "log");
    option_add("level", "4");
    option_add("output-service", "");
    channel_bind(MESSAGE_TYPE_LOGINFO, onloginfo);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

