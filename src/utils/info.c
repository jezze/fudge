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

    unsigned int service = fs_auth("system:");

    if (service)
    {

        unsigned int id = fs_walk(service, 0, "info/cores");

        if (id)
        {

            struct event_coreinfo coreinfo;
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(service, id, &coreinfo, sizeof (struct event_coreinfo), offset)); offset += count)
                channel_send_fmt3(source, EVENT_DATA, "core\n  id %u\n  sp 0x%H8u\n  task %u\n", &coreinfo.id, &coreinfo.sp, &coreinfo.task);

        }

    }

}

static void showtasks(unsigned int source)
{

    unsigned int service = fs_auth("system:");

    if (service)
    {

        unsigned int id = fs_walk(service, 0, "info/tasks");

        if (id)
        {


            struct event_taskinfo taskinfo;
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(service, id, &taskinfo, sizeof (struct event_taskinfo), offset)); offset += count)
            {

                if (taskinfo.state)
                    channel_send_fmt6(source, EVENT_DATA, "task\n  id %u\n  state %s\n  thread.ip 0x%H8u\n  thread.sp 0x%H8u\n  signals.kills %u\n  signals.blocks %u\n", &taskinfo.id, states[taskinfo.state], &taskinfo.thread_ip, &taskinfo.thread_sp, &taskinfo.signals_kills, &taskinfo.signals_blocks);

            }

        }

    }

}

static void showmailboxes(unsigned int source)
{

    unsigned int service = fs_auth("system:");

    if (service)
    {

        unsigned int id = fs_walk(service, 0, "info/mailboxes");

        if (id)
        {

            struct event_mailboxinfo mailboxinfo;
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(service, id, &mailboxinfo, sizeof (struct event_mailboxinfo), offset)); offset += count)
                channel_send_fmt1(source, EVENT_DATA, "mailbox\n  ring.buffer 0x%H8u\n", &mailboxinfo.address);

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

