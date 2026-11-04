#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    system_run(0, "sh -pwd initrd: initrd:data/sh/modules.sh");
    system_run(channel_lookup(option_getstring("console-service")), "sh -pwd initrd: initrd:data/sh/init.sh");

}

void init(void)
{

    option_add("console-service", "console:0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

