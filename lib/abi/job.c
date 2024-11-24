#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"
#include "job.h"

static char *nextword(char *current)
{

    return current + cstring_length_zero(current);

}

void job_parse(struct job *job, void *buffer, unsigned int count)
{

    char *start = buffer;
    char *end = start + count;

    while (start < end)
    {

        struct job_worker *p = &job->workers[job->count];

        switch (start[0])
        {

        case 'P':
            start = nextword(start);
            p->program = start;
            job->count++;

            break;

        case 'D':
            start = nextword(start);
            p->paths[p->npaths] = start;
            p->npaths++;

            break;

        case 'O':
            start = nextword(start);
            p->options[p->noptions].value = start;
            start = nextword(start);
            p->options[p->noptions].key = start;
            p->noptions++;

            break;

        case 'E':
            return;

        }

        start = nextword(start);

    }

}

static void activatenext(struct job *job, unsigned int ichannel, unsigned int startindex)
{

    unsigned int i;

    for (i = startindex; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
        {

            channel_send(ichannel, worker->target, EVENT_END);

            break;

        }

    }

}

unsigned int job_spawn(struct job *job, unsigned int ichannel, char *bindir)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        worker->target = fs_spawn_relative(ichannel, worker->program, bindir);

        if (!worker->target)
            worker->target = fs_spawn(ichannel, worker->program);

        if (!worker->target)
            return 0;

    }

    return job->count;

}

unsigned int job_pipe(struct job *job, unsigned int ichannel, unsigned int source, unsigned int event, void *buffer, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < job->count - 1; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target == source)
        {

            struct job_worker *next = &job->workers[i + 1];

            channel_send_buffer(ichannel, next->target, event, count, buffer);

            return 1;

        }

    }

    return 0;

}

void job_run(struct job *job, unsigned int ichannel, char *env, char *pwd)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
        {

            unsigned int j;

            channel_send_fmt2(ichannel, worker->target, EVENT_OPTION, "env\\0%s\\0pwd\\0%s\\0", env, pwd);

            for (j = 0; j < worker->noptions; j++)
                channel_send_fmt2(ichannel, worker->target, EVENT_OPTION, "%s\\0%s\\0", worker->options[j].key, worker->options[j].value);

        }

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
            channel_send(ichannel, worker->target, EVENT_MAIN);

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
        {

            unsigned int j;

            for (j = 0; j < worker->npaths; j++)
            {

                char *path = worker->paths[j];

                if (fs_auth(path))
                    channel_send_fmt1(ichannel, worker->target, EVENT_PATH, "%s\\0", path);
                else
                    channel_send_fmt2(ichannel, worker->target, EVENT_PATH, "%s%s\\0", pwd, path);

            }

        }

    }

    activatenext(job, ichannel, 0);

}

void job_close(struct job *job, unsigned int ichannel, unsigned int target)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target == target)
        {

            worker->target = 0;

            activatenext(job, ichannel, i + 1);

            break;

        }

    }

}

unsigned int job_exist(struct job *job, unsigned int target)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target == target)
            return target;

    }

    return 0;

}

unsigned int job_pick(struct job *job, unsigned int ichannel, struct message *message, unsigned int count, void *data)
{

    while (job_count(job) && channel_pick(0, message, count, data))
    {

        channel_dispatch(ichannel, message, data);

        if (message->event == EVENT_DONE)
            job_close(job, ichannel, message->source);

        if (job_exist(job, message->source))
            return message->source;

    }

    return 0;

}

void job_sendfirst(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
        {

            channel_send_buffer(ichannel, worker->target, event, count, buffer);

            break;

        }

    }

}

void job_sendall(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
            channel_send_buffer(ichannel, worker->target, event, count, buffer);

    }

}

void job_killall(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
        {

            call_kill(worker->target);

            worker->target = 0;

        }

    }

}

unsigned int job_count(struct job *job)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->target)
            count++;

    }

    return count;

}

void job_init(struct job *job, struct job_worker *workers, unsigned int capacity)
{

    job->workers = workers;
    job->capacity = capacity;
    job->count = 0;

    buffer_clear(job->workers, sizeof (struct job_worker) * capacity);

}

