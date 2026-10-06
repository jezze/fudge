#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    system_run(0, "mq -query .modules.path initrd:data/config/modules.mq | elfload");
    system_run(0, "shell -pwd initrd: -keyboard-service keyboard:1 &");
    system_run(0, "automount -pwd initrd: &");
    system_run(0, "wm -pwd initrd: &");

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}
