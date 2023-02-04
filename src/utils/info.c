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
        unsigned int n = 0;

        for (n = 0; file_read(FILE_L0, &ctrl, sizeof (struct ctrl_core)); n++)
        {

            channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "core[%u] {id=%u}\n", &n, &ctrl.id);

            if (n >= 9)
                break;

        }

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
        unsigned int n = 0;

        for (n = 0; file_read(FILE_L0, &ctrl, sizeof (struct ctrl_task)); n++)
        {

            if (ctrl.state)
            {

                channel_sendfmt3(CHANNEL_DEFAULT, EVENT_DATA, "task[%u] {id=%u, state=%s, ", &n, &ctrl.id, states[ctrl.state]);
                channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "thread.ip=0x%H8u, thread.sp=0x%H8u, ", &ctrl.thread_ip, &ctrl.thread_sp);
                channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "signals.kills=%u, signals.blocks=%u}\n", &ctrl.signals_kills, &ctrl.signals_blocks);

            }

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
        unsigned int n = 0;

        for (n = 0; file_read(FILE_L0, &ctrl, sizeof (struct ctrl_mailbox)); n++)
        {

            channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "mailbox[%u] {ring.buffer=0x%H8u}\n", &n, &ctrl.address);

            if (n >= 9)
                break;

        }

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

