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
            message_putstring(&message, "core[");
            message_putvalue(&message, n, 10, 0);
            message_putstring(&message, "] {");
            message_putstring(&message, "id=");
            message_putvalue(&message, ctrl.id, 10, 0);
            message_putstring(&message, "}\n");
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

            struct message message;

            message_init(&message, EVENT_DATA);
            message_putstring(&message, "task[");
            message_putvalue(&message, n, 10, 0);
            message_putstring(&message, "] {");
            message_putstring(&message, "id=");
            message_putvalue(&message, ctrl.id, 10, 0);
            message_putstring(&message, ", ");
            message_putstring(&message, "state=");
            message_putvalue(&message, ctrl.state, 10, 0);
            message_putstring(&message, ", ");
            message_putstring(&message, "thread.ip=0x");
            message_putvalue(&message, ctrl.thread_ip, 16, 8);
            message_putstring(&message, ", ");
            message_putstring(&message, "thread.sp=0x");
            message_putvalue(&message, ctrl.thread_sp, 16, 8);
            message_putstring(&message, ", ");
            message_putstring(&message, "signals.kills=");
            message_putvalue(&message, ctrl.signals_kills, 10, 0);
            message_putstring(&message, ", ");
            message_putstring(&message, "signals.blocks=");
            message_putvalue(&message, ctrl.signals_blocks, 10, 0);
            message_putstring(&message, ", ");
            message_putstring(&message, "signals.unblocks=");
            message_putvalue(&message, ctrl.signals_unblocks, 10, 0);
            message_putstring(&message, "}\n");
            channel_sendmessage(&message);

            if (n >= 9)
                break;

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
            message_putstring(&message, "mailbox[");
            message_putvalue(&message, n, 10, 0);
            message_putstring(&message, "] {");
            message_putstring(&message, "ring.buffer=0x");
            message_putvalue(&message, ctrl.address, 16, 8);
            message_putstring(&message, "}\n");
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

