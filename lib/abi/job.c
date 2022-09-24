#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

static char *nextword(char *current)
{

    return current + cstring_lengthz(current);

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

static unsigned int spawn(struct job_worker *worker)
{

    if (!file_walk2(FILE_L0, "/bin"))
        return 0;

    if (!(file_walk(FILE_CP, FILE_L0, worker->program) || file_walk2(FILE_CP, worker->program)))
        return 0;

    return call_spawn();

}

void activatenext(unsigned int startindex, struct job *job)
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

void job_spawn(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        worker->id = spawn(worker);

        if (!worker->id)
        {

            job_sendall(job, EVENT_TERM, 0, 0);

            return;

        }

    }

}

void job_pipe(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
        {

            channel_redirectback(worker->id, EVENT_CLOSE);
            channel_redirectback(worker->id, EVENT_PATH);
            channel_redirectback(worker->id, EVENT_ERROR);

            if (i < job->count - 1)
                channel_redirecttarget(worker->id, EVENT_DATA, job->workers[i + 1].id);
            else
                channel_redirectback(worker->id, EVENT_DATA);

        }

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
            {

                struct message message;

                message_init(&message, EVENT_OPTION);
                message_putstringz(&message, worker->options[j].key);
                message_putstringz(&message, worker->options[j].value);
                channel_sendbufferto(worker->id, EVENT_OPTION, message_datasize(&message.header), message.data.buffer);

            }

        }

    }

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id)
        {

            unsigned int j;

            for (j = 0; j < worker->npaths; j++)
                channel_sendstringzto(worker->id, EVENT_PATH, worker->paths[j]);

        }

    }

    activatenext(0, job);

}

void job_close(unsigned int id, struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->count; i++)
    {

        struct job_worker *worker = &job->workers[i];

        if (worker->id == id)
        {

            worker->id = 0;

            activatenext(i + 1, job);

            break;

        }

    }

}

void job_send(struct job *job, unsigned int event, unsigned int count, void *buffer)
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

