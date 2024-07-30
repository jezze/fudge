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

static void showcores(unsigned int source)
{

    unsigned int service = fsp_auth("system:");

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, "info/cores");

        if (id)
        {

            struct ctrl_core ctrl;
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fsp_read(service, id, &ctrl, sizeof (struct ctrl_core), offset)); offset += count)
                channel_send_fmt3(source, EVENT_DATA, "core\n  id %u\n  sp 0x%H8u\n  task %u\n", &ctrl.id, &ctrl.sp, &ctrl.task);

        }

    }

}

static void showtasks(unsigned int source)
{

    unsigned int service = fsp_auth("system:");

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, "info/tasks");

        if (id)
        {


            struct ctrl_task ctrl;
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fsp_read(service, id, &ctrl, sizeof (struct ctrl_task), offset)); offset += count)
            {

                if (ctrl.state)
                    channel_send_fmt6(source, EVENT_DATA, "task\n  id %u\n  state %s\n  thread.ip 0x%H8u\n  thread.sp 0x%H8u\n  signals.kills %u\n  signals.blocks %u\n", &ctrl.id, states[ctrl.state], &ctrl.thread_ip, &ctrl.thread_sp, &ctrl.signals_kills, &ctrl.signals_blocks);

            }

        }

    }

}

static void showmailboxes(unsigned int source)
{

    unsigned int service = fsp_auth("system:");

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, "info/mailboxes");

        if (id)
        {

            struct ctrl_mailbox ctrl;
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fsp_read(service, id, &ctrl, sizeof (struct ctrl_mailbox), offset)); offset += count)
                channel_send_fmt1(source, EVENT_DATA, "mailbox\n  ring.buffer 0x%H8u\n", &ctrl.address);

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    showcores(source);
    showtasks(source);
    showmailboxes(source);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

