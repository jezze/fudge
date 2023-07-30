#include <fudge.h>
#include "call.h"
#include "channel.h"
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

            channel_send(worker->channel, EVENT_MAIN);

            return;

        }

    }

}

unsigned int job_spawn(struct job *job, unsigned int pdescriptor, unsigned int wdescriptor)
{

    unsigned int i;

    if (!call_walk_absolute(FILE_L0, "/bin"))
    {

        channel_send_fmt0(CHANNEL_DEFAULT, EVENT_ERROR, "Bin directory not found\n");

        return 0;

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (!(call_walk_relative(pdescriptor, FILE_L0, worker->program) || call_walk_absolute(pdescriptor, worker->program)))
        {

            channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", &worker->program);

            return 0;

        }

        worker->channel = call_spawn(pdescriptor, wdescriptor);

        if (!worker->channel)
        {

            channel_send_fmt0(CHANNEL_DEFAULT, EVENT_ERROR, "Job spawn failed\n");

            return 0;

        }

    }

    return job->count;

}

void job_listen(struct job *job, unsigned int event)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
            channel_listen(worker->channel, event);

    }

}

void job_pipe(struct job *job, unsigned int event)
{

    unsigned int i;

    for (i = 0; i < job->count - 1; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
            channel_forward(worker->channel, event, job->workers[i + 1].channel);

    }

}

void job_run(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
        {

            unsigned int j;

            for (j = 0; j < worker->noptions; j++)
                channel_send_fmt2(worker->channel, EVENT_OPTION, "%s\\0%s\\0", worker->options[j].key, worker->options[j].value);

        }

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->channel)
        {

            unsigned int j;

            for (j = 0; j < worker->npaths; j++)
                channel_send_fmt1(worker->channel, EVENT_PATH, "%s\\0", worker->paths[j]);

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

unsigned int job_pick(struct job *job, struct message *message, void *data)
{

    while (job_count(job) && channel_pick(message, data))
    {

        if (job_exist(job, message->source))
            return message->source;

        channel_dispatch(message, data);

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

