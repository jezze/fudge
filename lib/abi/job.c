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

static void activatenext(struct job *job, unsigned int startindex)
{

    unsigned int i;

    for (i = startindex; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
        {

            channel_send(worker->channel, EVENT_END);

            break;

        }

    }

}

unsigned int job_spawn(struct job *job, char *bindir)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        worker->channel = fs_spawn_relative(worker->program, bindir);

        if (!worker->channel)
            worker->channel = fs_spawn(worker->program);

        if (!worker->channel)
            return 0;

    }

    return job->count;

}

unsigned int job_pipe(struct job *job, unsigned int source, unsigned int event, void *buffer, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < job->count - 1; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel == source)
        {

            struct job_worker *next = &job->workers[i + 1];

            channel_send_buffer(next->channel, event, count, buffer);

            return 1;

        }

    }

    return 0;

}

void job_run(struct job *job, char *pwd)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
        {

            unsigned int j;

            channel_send_fmt1(worker->channel, EVENT_OPTION, "pwd\\0%s\\0", pwd);

            for (j = 0; j < worker->noptions; j++)
                channel_send_fmt2(worker->channel, EVENT_OPTION, "%s\\0%s\\0", worker->options[j].key, worker->options[j].value);

        }

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
            channel_send(worker->channel, EVENT_MAIN);

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
        {

            unsigned int j;

            for (j = 0; j < worker->npaths; j++)
            {

                char *path = worker->paths[j];

                if (fs_auth(path))
                    channel_send_fmt1(worker->channel, EVENT_PATH, "%s\\0", path);
                else
                    channel_send_fmt2(worker->channel, EVENT_PATH, "%s%s\\0", pwd, path);

            }

        }

    }

    activatenext(job, 0);

}

void job_close(struct job *job, unsigned int channel)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel == channel)
        {

            worker->channel = 0;

            activatenext(job, i + 1);

            break;

        }

    }

}

unsigned int job_exist(struct job *job, unsigned int channel)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel == channel)
            return channel;

    }

    return 0;

}

unsigned int job_pick(struct job *job, struct message *message, unsigned int count, void *data)
{

    while (job_count(job) && channel_pick(message, count, data))
    {

        channel_dispatch(message, data);

        if (message->event == EVENT_TERMRESPONSE)
            job_close(job, message->source);

        return message->source;

    }

    return 0;

}

void job_sendfirst(struct job *job, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
        {

            channel_send_buffer(worker->channel, event, count, buffer);

            break;

        }

    }

}

void job_sendall(struct job *job, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
            channel_send_buffer(worker->channel, event, count, buffer);

    }

}

void job_killall(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
        {

            call_kill(worker->channel);

            worker->channel = 0;

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

        if (worker->channel)
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

