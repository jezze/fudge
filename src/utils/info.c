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

    if (call_walk_absolute(FILE_L0, "system:info/cores"))
    {

        struct ctrl_core ctrl;
        unsigned int count;
        unsigned int offset;

        for (offset = 0; (count = call_read(FILE_L0, &ctrl, sizeof (struct ctrl_core), offset)); offset += count)
            channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "core\n  id %u\n  sp 0x%H8u\n  task %u\n", &ctrl.id, &ctrl.sp, &ctrl.task);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Cores not found: %s\n", "system:info/cores");

    }

}

static void showtasks(void)
{

    if (call_walk_absolute(FILE_L0, "system:info/tasks"))
    {

        struct ctrl_task ctrl;
        unsigned int count;
        unsigned int offset;

        for (offset = 0; (count = call_read(FILE_L0, &ctrl, sizeof (struct ctrl_task), offset)); offset += count)
        {

            if (ctrl.state)
                channel_send_fmt6(CHANNEL_DEFAULT, EVENT_DATA, "task\n  id %u\n  state %s\n  thread.ip 0x%H8u\n  thread.sp 0x%H8u\n  signals.kills %u\n  signals.blocks %u\n", &ctrl.id, states[ctrl.state], &ctrl.thread_ip, &ctrl.thread_sp, &ctrl.signals_kills, &ctrl.signals_blocks);

        }

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Tasks not found: %s\n", "system:info/tasks");

    }

}

static void showmailboxes(void)
{

    if (call_walk_absolute(FILE_L0, "system:info/mailboxes"))
    {

        struct ctrl_mailbox ctrl;
        unsigned int count;
        unsigned int offset;

        for (offset = 0; (count = call_read(FILE_L0, &ctrl, sizeof (struct ctrl_mailbox), offset)); offset += count)
            channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "mailbox\n  ring.buffer 0x%H8u\n", &ctrl.address);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Mailboxes not found: %s\n", "system:info/mailboxes");

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    showcores();
    showtasks();
    showmailboxes();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

