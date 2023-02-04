#include <fudge.h>
#include <abi.h>

static char *states[6] = {
    "DEAD",
    "NEW",
    "BLOCKED",
    "UNBLOCKED",
    "ASSIGNED",
    "RUNNING"
};

static void showcores(void)
{

    if (file_walk2(FILE_L0, "system:info/cores"))
    {

        struct ctrl_core ctrl;

        while (file_read(FILE_L0, &ctrl, sizeof (struct ctrl_core)))
            channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "core\n  id %u\n", &ctrl.id);

    }

    else
    {

        channel_error("Cores not found.");

    }

}

static void showtasks(void)
{

    if (file_walk2(FILE_L0, "system:info/tasks"))
    {

        struct ctrl_task ctrl;

        while (file_read(FILE_L0, &ctrl, sizeof (struct ctrl_task)))
        {

            if (ctrl.state)
                channel_sendfmt6(CHANNEL_DEFAULT, EVENT_DATA, "task\n  id %u\n  state %s\n  thread.ip 0x%H8u\n  thread.sp 0x%H8u\n  signals.kills %u\n  signals.blocks %u\n", &ctrl.id, states[ctrl.state], &ctrl.thread_ip, &ctrl.thread_sp, &ctrl.signals_kills, &ctrl.signals_blocks);

        }

    }

    else
    {

        channel_error("Tasks not found.");

    }

}

static void showmailboxes(void)
{

    if (file_walk2(FILE_L0, "system:info/mailboxes"))
    {

        struct ctrl_mailbox ctrl;

        while (file_read(FILE_L0, &ctrl, sizeof (struct ctrl_mailbox)))
            channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "mailbox\n  ring.buffer 0x%H8u\n", &ctrl.address);

    }

    else
    {

        channel_error("Mailboxes not found.");

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    showcores();
    showtasks();
    showmailboxes();
    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

