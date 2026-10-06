#include <fudge.h>
#include <abi.h>

static unsigned int readmodules(unsigned int ichannel, char *paths, unsigned int size)
{

    unsigned int target = fs_spawn(ichannel, ichannel, "initrd:bin/mq");
    unsigned int count = 0;

    if (target)
    {

        struct message message;
        unsigned int i;

        channel_send_fmt0(ichannel, target, EVENT_OPTION, "query=.modules.path\n");
        channel_send(ichannel, target, EVENT_MAIN, 0, 0);
        channel_send_fmt0(ichannel, target, EVENT_PATH, "initrd:data/config/modules.mq\\0");
        channel_send(ichannel, target, EVENT_TERM, 0, 0);

        while (channel_poll(ichannel, target, EVENT_DATA, &message))
            count += buffer_write(paths, size, message.data, message.length, count);

        for (i = 0; i < count; i++)
        {

            if (paths[i] == '\n')
                paths[i] = '\0';

        }

    }

    return count;

}

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

    loadmodules(1, readmodules(1, paths, MESSAGE_SIZE), paths);
    spawnshell(1);
    spawnautomount(1);
    spawnwm(1);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

