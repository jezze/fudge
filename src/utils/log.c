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
        union {struct event_loginfo loginfo; char data[MESSAGE_SIZE];} data;

        call_link(FILE_L0, 8000);

        while (channel_poll_any(EVENT_LOGINFO, &message, &data))
        {

            char *description = data.data + sizeof (struct event_loginfo);
            unsigned int count = data.loginfo.count - sizeof (struct event_loginfo);

            if (option_getdecimal("level") >= data.loginfo.level)
                channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "[%s] %w\n", levels[data.loginfo.level], description, &count);

        }

        call_unlink(FILE_L0);

    }

    else
    {

        channel_send_fmt2(CHANNEL_DEFAULT, EVENT_ERROR, "Logs not found: %s and/or %s\n", option_getstring("klog"), option_getstring("ulog"));

    }

}

void init(void)
{

    option_add("klog", "system:log/messages");
    option_add("level", "4");
    channel_bind(EVENT_MAIN, onmain);

}

