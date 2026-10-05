#include <fudge.h>
#include <abi.h>

#define BINPATH                         "initrd:bin"
#define SCRIPTSIZE                      0x4000

static struct job job;
static char script[SCRIPTSIZE];
static unsigned int scriptcount;
static unsigned int output;
static unsigned int running;

static void run(char *pwd)
{

    unsigned int offset = 0;

    while (offset < scriptcount)
    {

        if (!job_parse(&job, script, scriptcount, &offset))
        {

            channel_send_fmt1(0, output, EVENT_ERROR, "%s\n", job.error);

            return;

        }

        if (!job.ncommands)
            continue;

        if (!job_spawn(&job, 1, 0, BINPATH))
        {

            channel_send_fmt1(0, output, EVENT_ERROR, "%s\n", job.error);
            job_abort(&job, 0);

            continue;

        }

        job_run(&job, 0, pwd);

        while (job_count(&job))
        {

            if (!channel_process(0))
            {

                job_sendall(&job, 0, EVENT_INTERRUPT, 0, 0);

                return;

            }

        }

    }

}

static void append(void *data, unsigned int count)
{

    scriptcount += buffer_write(script, SCRIPTSIZE - 1, data, count, scriptcount);

}

static void onconsoledata(struct message *message)
{

    job_sendfirst(&job, 0, EVENT_CONSOLEDATA, message->length, message->data);

}

static void ondata(struct message *message)
{

    if (job_exist(&job, message->source))
        channel_send(0, output, EVENT_DATA, message->length, message->data);
    else if (running)
        job_sendfirst(&job, 0, EVENT_DATA, message->length, message->data);
    else
        append(message->data, message->length);

}

static void onerror(struct message *message)
{

    if (job_exist(&job, message->source))
        channel_send(0, output, EVENT_ERROR, message->length, message->data);

}

static void onexit(struct message *message)
{

    struct event_exit *exit = message->data;

    job_exit(&job, 0, message->source, exit->status);

}

static void onmain(struct message *message)
{

    output = message->source;

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);
    unsigned int id = (target) ? fs_walk(1, target, 0, message->data) : 0;
    char buffer[0x800];
    unsigned int offset = 0;
    unsigned int count;

    if (!id)
    {

        channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        return;

    }

    while ((count = fs_read(1, target, id, buffer, 0x800, offset)))
    {

        append(buffer, count);

        offset += count;

    }

    append("\n", 1);

}

static void onterm(struct message *message)
{

    running = 1;

    run(option_getstring("pwd"));

    if (option_getdecimal("export"))
        channel_send_fmt1(0, output, EVENT_OPTION, "pwd=%s\n", option_getstring("pwd"));

}

void init(void)
{

    option_add("pwd", "initrd:");
    option_add("export", "0");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

