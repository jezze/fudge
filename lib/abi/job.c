#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

static char *nextword(char *current)
{

    return current + cstring_lengthzero(current);

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

        if (worker->id)
        {

            channel_sendto(worker->id, EVENT_MAIN);

            return;

        }

    }

}

unsigned int job_spawn(struct job *job, unsigned int pdescriptor, unsigned int wdescriptor)
{

    unsigned int i;

    if (!file_walk2(FILE_L0, "/bin"))
        return 0;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (!(file_walk(pdescriptor, FILE_L0, worker->program) || file_walk2(pdescriptor, worker->program)))
            return 0;

        worker->id = call_spawn(pdescriptor, wdescriptor);

        if (!worker->id)
            return 0;

    }

    return job->count;

}

void job_listen(struct job *job, unsigned int event)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
            channel_redirectback(worker->id, event);

    }

}

void job_pipe(struct job *job, unsigned int event)
{

    unsigned int i;

    for (i = 0; i < job->count - 1; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
            channel_redirecttarget(worker->id, event, job->workers[i + 1].id);

    }

}

void job_run(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
        {

            unsigned int j;

            for (j = 0; j < worker->noptions; j++)
                channel_sendfmt2to(worker->id, EVENT_OPTION, "%s\\0%s\\0", worker->options[j].key, worker->options[j].value);

        }

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
        {

            unsigned int j;

            for (j = 0; j < worker->npaths; j++)
                channel_sendfmt1to(worker->id, EVENT_PATH, "%s\\0", worker->paths[j]);

        }

    }

    activatenext(job, 0);

}

void job_close(struct job *job, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id == id)
        {

            worker->id = 0;

            activatenext(job, i + 1);

            break;

        }

    }

}

unsigned int job_exist(struct job *job, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id == id)
            return id;

    }

    return 0;

}

unsigned int job_pick(struct job *job, struct message_header *header, unsigned int count, void *data)
{

    while (job_count(job) && channel_pick(header, count, data))
    {

        if (job_exist(job, header->source))
            return header->source;
        else
            channel_dispatch(header, data);

    }

    return 0;

}

void job_sendfirst(struct job *job, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
        {

            channel_sendbufferto(worker->id, event, count, buffer);

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

        if (worker->id)
            channel_sendbufferto(worker->id, event, count, buffer);

    }

}

void job_killall(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
        {

            call_kill(worker->id);

            worker->id = 0;

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

        if (worker->id)
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

