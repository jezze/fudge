#include <fudge.h>
#include <abi.h>

static void loadmodules(unsigned int ichannel, unsigned int count, char *paths)
{

    unsigned int target = fs_spawn(ichannel, ichannel, "initrd:bin/elfload");

    if (target)
    {

        channel_send(ichannel, target, EVENT_MAIN, 0, 0);
        channel_send(ichannel, target, EVENT_PATH, count, paths);
        channel_send(ichannel, target, EVENT_TERM, 0, 0);
        channel_wait(ichannel, target, EVENT_EXIT, 0, 0);

    }

}

static unsigned int spawnshell(unsigned int ichannel)
{

    unsigned int target = fs_spawn(ichannel, ichannel, "initrd:bin/shell");

    if (target)
    {

        channel_send_fmt0(ichannel, target, EVENT_OPTION, "pwd=initrd:&keyboard-service=keyboard:1\n");
        channel_send(ichannel, target, EVENT_MAIN, 0, 0);

    }

    return target;

}

static unsigned int spawnautomount(unsigned int ichannel)
{

    unsigned int target = fs_spawn(ichannel, ichannel, "initrd:bin/automount");

    if (target)
    {

        channel_send_fmt0(ichannel, target, EVENT_OPTION, "pwd=initrd:\n");
        channel_send(ichannel, target, EVENT_MAIN, 0, 0);
        channel_send(ichannel, target, EVENT_TERM, 0, 0);

    }

    return target;

}

static unsigned int spawnwm(unsigned int ichannel)
{

    unsigned int target = fs_spawn(ichannel, ichannel, "initrd:bin/wm");

    if (target)
    {

        channel_send_fmt0(ichannel, target, EVENT_OPTION, "pwd=initrd:\n");
        channel_send(ichannel, target, EVENT_MAIN, 0, 0);

    }

    return target;

}

static void onmain(struct message *message)
{

    char paths[MESSAGE_SIZE];
    unsigned int count = system_read("mq -query .modules.path initrd:data/config/modules.mq | tr -f '\\n' -t '\\0'", paths, MESSAGE_SIZE);

    loadmodules(1, count, paths);
    spawnshell(1);
    spawnautomount(1);
    spawnwm(1);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

