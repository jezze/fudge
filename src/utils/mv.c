#include <fudge.h>
#include <abi.h>

static char from[1024];
static unsigned int paths;

static void onpath(struct message *message)
{

    if (!paths)
        buffer_write(from, 1024, message->data, cstring_length_zero(message->data), 0);
    else if (paths == 1 && system_run(message->source, "cp \"%s\" \"%s\"", from, message->data) == EXIT_STATUS_NORMAL)
        system_run(message->source, "rm \"%s\"", from);

    paths++;

}

static void onterm(struct message *message)
{

    if (paths != 2)
        channel_send_fmt(0, message->source, EVENT_ERROR, "Usage: mv <source> <destination>\n");

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

