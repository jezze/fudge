#include <fudge.h>
#include <abi.h>

#define TESTS                           14
#define PATHSIZE                        64
#define DEADLINE                        30000
#define GRACE                           5000
#define RESULT_OK                       0
#define RESULT_FAIL                     1
#define RESULT_HANG                     2

struct test
{

    char *name;
    char *known;

};

/* known: why the test is expected to fail; a known test that passes is reported so it can be taken off; tests that count the whole pool come before audio, which keeps pages while it plays, and tests that may leave tasks holding pages go last */
static struct test tests[TESTS] = {
    {"spawn", 0},
    {"pingpong", 0},
    {"flood", 0},
    {"window0", "relieve runs a newer message inside an older one's retrying send, so replies overtake"},
    {"window1", "replies piling up in channel 1 are not relieved while its task sends"},
    {"abort", 0},
    {"stale", 0},
    {"pages", 0},
    {"pipe", 0},
    {"cut", 0},
    {"pipes", 0},
    {"relay", 0},
    {"audio", 0},
    {"expand", 0}
};

static unsigned int output;
static unsigned int timer;

static void arm(unsigned int id, unsigned int ms)
{

    struct message_timerrequest timerrequest;

    timerrequest.id = id;
    timerrequest.ms = ms;
    timerrequest.repeat = 0;

    if (timer)
        channel_send(1, timer, MESSAGE_TYPE_TIMERREQUEST, sizeof (struct message_timerrequest), &timerrequest);

}

static unsigned int expired(struct message *message, unsigned int id)
{

    struct message_timerexpire *timerexpire = message_validate(MESSAGE_TYPE_TIMEREXPIRE, message->data, message->length);

    return timer && message->source == timer && timerexpire && timerexpire->id == id;

}

/* runs sh, which waits on channel 1 and drops other mail there; the deadline is armed again afterwards */
static void dump(void)
{

    system_run(output, "mq -query .tasks -format \"dump task {id} {state}\" sysinfo:tasks.mq");
    system_run(output, "mq -query . -format \"dump pages free {free} of {count}\" sysinfo:pages.mq");
    system_run(output, "mq -query .mailboxes -format \"dump mailbox {inode} task {itask} channel {ichannel} messages {nmessages}\" sysinfo:mailboxes.mq");

}

static unsigned int run(unsigned int id, char *name)
{

    char path[PATHSIZE];
    unsigned int strikes = 0;
    unsigned int target;
    struct message message;

    cstring_write_fmt(path, PATHSIZE, 0, "initrd:test/%s\\0", name);
    channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s start\n", name);

    target = fs_spawn(1, path);

    if (!target)
    {

        channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s: spawn failed\n", name);

        return RESULT_FAIL;

    }

    channel_send(1, target, MESSAGE_TYPE_MAIN, 0, 0);
    channel_send(1, target, MESSAGE_TYPE_TERM, 0, 0);
    arm(id, DEADLINE);

    while (channel_pick(1, &message))
    {

        struct message_status *status;

        if (message.type == MESSAGE_TYPE_TIMEREXPIRE && expired(&message, id))
        {

            if (strikes++)
            {

                call_kill(target);
                channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s: did not abort, killed\n", name);

                return RESULT_HANG;

            }

            channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s: deadline passed, mailbox %u, aborting\n", name, &target);
            dump();
            call_kill(target);
            arm(id, GRACE);

            continue;

        }

        if (message.source != target)
            continue;

        switch (message.type)
        {

        case MESSAGE_TYPE_DATA:
        case MESSAGE_TYPE_ERROR:
            channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s: %w", name, message.data, &message.length);

            break;

        case MESSAGE_TYPE_STATUS:
            status = message_validate(MESSAGE_TYPE_STATUS, message.data, message.length);

            arm(id, 0);

            if (strikes)
                return RESULT_HANG;

            return (status && status->code == MESSAGE_STATUS_OK) ? RESULT_OK : RESULT_FAIL;

        }

    }

    return RESULT_FAIL;

}

static void onmain(struct message *message)
{

    unsigned int passed = 0;
    unsigned int failed = 0;
    unsigned int known = 0;
    unsigned int i;

    output = channel_lookup("console:0");
    timer = channel_lookup("timer:0");

    channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test boot ok\n");

    if (!timer)
        channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test runner: no timer:0, tests have no deadline\n");

    for (i = 0; i < TESTS; i++)
    {

        unsigned int result = run(i + 1, tests[i].name);

        if (tests[i].known)
        {

            if (result == RESULT_OK)
            {

                channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s fixed, take it off the known list\n", tests[i].name);

                failed++;

            }

            else
            {

                channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s known: %s\n", tests[i].name, tests[i].known);

                known++;

            }

        }

        else if (result == RESULT_OK)
        {

            channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s ok\n", tests[i].name);

            passed++;

        }

        else
        {

            channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "test %s %s\n", tests[i].name, (result == RESULT_HANG) ? "hang" : "fail");

            failed++;

        }

    }

    channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "tests done %u passed %u failed %u known\n", &passed, &failed, &known);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

