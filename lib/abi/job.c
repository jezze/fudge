#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

static char *nextword(char *current)
{

    return current + ascii_lengthz(current);

}

unsigned int job_parse(struct job *jobs, unsigned int n, void *buffer, unsigned int count)
{

    char *start = buffer;
    char *end = start + count;
    unsigned int njobs = 0;

    buffer_clear(jobs, sizeof (struct job) * n);

    while (start < end)
    {

        struct job *p = &jobs[njobs];

        switch (start[0])
        {

        case 'P':
            start = nextword(start);
            p->program = start;
            njobs++;

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
            start = nextword(start);

            return njobs;

        }

        start = nextword(start);

    }

    return njobs;

}

static unsigned int spawn(struct job *job)
{

    if (!file_walk2(FILE_L0, "/bin"))
        return 0;

    if (!(file_walk(FILE_CP, FILE_L0, job->program) || file_walk2(FILE_CP, job->program)))
        return 0;

    return call_spawn();

}

void activatenext(struct channel *channel, unsigned int index, struct job *jobs, unsigned int n)
{

    if (index < n)
    {

        struct job *job = &jobs[index];

        if (job->id)
            channel_send(channel, job->id, EVENT_MAIN);

    }

}

unsigned int job_spawn(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        job->id = spawn(job);

        if (!job->id)
        {

            job_send(channel, jobs, i, EVENT_TERM, 0, 0);

            return 0;

        }

    }

    return n;

}

void job_pipe(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            channel_sendredirectback(channel, job->id, EVENT_CLOSE);
            channel_sendredirectback(channel, job->id, EVENT_PATH);

            if (i < n - 1)
                channel_sendredirectto(channel, job->id, EVENT_DATA, jobs[i + 1].id);
            else
                channel_sendredirectback(channel, job->id, EVENT_DATA);

        }

    }

}

unsigned int job_run(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            unsigned int j;
            struct message_data data;

            for (j = 0; j < job->noptions; j++)
                channel_sendbuffer(channel, job->id, EVENT_OPTION, message_putstringz(&data, job->options[j].value, message_putstringz(&data, job->options[j].key, 0)), &data);

        }

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            unsigned int j;

            for (j = 0; j < job->npaths; j++)
                channel_sendstringz(channel, job->id, EVENT_PATH, job->paths[j]);

        }

    }

    activatenext(channel, 0, jobs, n);

    return job_count(channel, jobs, n);

}

unsigned int job_close(struct channel *channel, unsigned int id, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id == id)
        {

            job->id = 0;

            activatenext(channel, i + 1, jobs, n);

            break;

        }

    }

    return job_count(channel, jobs, n);

}

void job_send(struct channel *channel, struct job *jobs, unsigned int n, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            channel_sendbuffer(channel, job->id, event, count, buffer);

        break;

    }

}

void job_kill(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            call_kill(job->id);

    }

}

unsigned int job_count(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            count++;

    }

    return count;

}

