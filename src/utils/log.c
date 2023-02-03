#include <fudge.h>
#include <abi.h>

static char *names[5] = {
    "NULL",
    "CRIT",
    "ERRO",
    "WARN",
    "INFO"
};

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, option_getstring("log")))
    {

        struct message message;
        struct {struct event_loginfo loginfo; char buffer[200]; } data;

        file_link(FILE_L0);

        while (channel_kpollevent(EVENT_LOGINFO, &message, sizeof (struct event_loginfo) + 200, &data))
        {

            if (option_getdecimal("level") >= data.loginfo.level)
                channel_sendfmt3(EVENT_DATA, "[%s] %w\n", names[data.loginfo.level], data.buffer, &data.loginfo.count);

        }

        file_unlink(FILE_L0);

    }

    else
    {

        channel_error("Log not found");

    }

    channel_close();

}

void init(void)
{

    option_add("log", "system:log/messages");
    option_add("level", "4");
    channel_bind(EVENT_MAIN, onmain);

}

