#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    while (system_run(message->source, "%s", option_getstring("command")));

}

void init(void)
{

    option_add("command", "");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

