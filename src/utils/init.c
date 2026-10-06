#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    system_run(0, "sh -pwd initrd: initrd:data/sh/init.sh");

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}
