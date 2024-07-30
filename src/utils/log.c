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

    unsigned int service = fsp_auth(option_getstring("klog"));

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, option_getstring("klog"));

        if (id)
        {

            struct message message;
            char data[MESSAGE_SIZE];

            fsp_link(service, id);

            while (channel_poll_any(EVENT_LOGINFO, &message, MESSAGE_SIZE, data))
            {

                struct event_loginfo *loginfo = (void *)data;
                char *description = (char *)(loginfo + 1);
                unsigned int count = loginfo->count - sizeof (struct event_loginfo);

                if (option_getdecimal("level") >= loginfo->level)
                    channel_send_fmt3(source, EVENT_DATA, "[%s] %w\n", levels[loginfo->level], description, &count);

            }

            fsp_unlink(service, id);

        }

    }

}

void init(void)
{

    option_add("klog", "system:log/messages");
    option_add("level", "4");
    channel_bind(EVENT_MAIN, onmain);

}

