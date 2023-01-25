#include <fudge.h>
#include <abi.h>

static void showcores(void)
{

    if (file_walk2(FILE_L0, "system:info/cores"))
    {

        struct ctrl_core ctrl;
        unsigned int n = 0;

        for (n = 0; file_read(FILE_L0, &ctrl, sizeof (struct ctrl_core)); n++)
        {

            struct message message;

            message_init(&message, EVENT_DATA);
            message_putfmt2(&message, "core[%u] {id=%u}\n", &n, &ctrl.id);
            channel_sendmessage(&message);

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

                struct message message;

                message_init(&message, EVENT_DATA);
                message_putfmt3(&message, "task[%u] {id=%u, state=%u, ", &n, &ctrl.id, &ctrl.state);
                message_putfmt2(&message, "thread.ip=0x%H8u, thread.sp=0x%H8u, ", &ctrl.thread_ip, &ctrl.thread_sp);
                message_putfmt3(&message, "signals.kills=%u, signals.blocks=%u, signals.unblocks=%u}\n", &ctrl.signals_kills, &ctrl.signals_blocks, &ctrl.signals_unblocks);
                channel_sendmessage(&message);

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

            struct message message;

            message_init(&message, EVENT_DATA);
            message_putfmt2(&message, "mailbox[%u] {ring.buffer=0x%H8u}\n", &n, &ctrl.address);
            channel_sendmessage(&message);

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

