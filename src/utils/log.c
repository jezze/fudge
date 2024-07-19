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

    if (call_walk_absolute(FILE_L0, option_getstring("klog")))
    {

        struct message message;
        char data[MESSAGE_SIZE];

        call_link(FILE_L0, 8000);

        while (channel_poll_any(EVENT_LOGINFO, &message, MESSAGE_SIZE, data))
        {

            struct event_loginfo *loginfo = (void *)data;
            char *description = (char *)(loginfo + 1);
            unsigned int count = loginfo->count - sizeof (struct event_loginfo);

            if (option_getdecimal("level") >= loginfo->level)
                channel_send_fmt3(source, EVENT_DATA, "[%s] %w\n", levels[loginfo->level], description, &count);

        }

        call_unlink(FILE_L0);

    }

    else
    {

        channel_send_fmt2(source, EVENT_ERROR, "Logs not found: %s and/or %s\n", option_getstring("klog"), option_getstring("ulog"));

    }

}

void init(void)
{

    option_add("klog", "system:log/messages");
    option_add("level", "4");
    channel_bind(EVENT_MAIN, onmain);

}

