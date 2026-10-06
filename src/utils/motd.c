#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    system_run(message->source, "echo initrd:data/motd.txt");

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}
