#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

static char *nextword(char *current)
{

    return current + cstring_lengthz(current);

}

unsigned int job_parse(struct job *jobs, unsigned int c, unsigned int n, void *buffer, unsigned int count)
{

    char *start = buffer;
    char *end = start + count;

    buffer_clear(jobs, sizeof (struct job) * n);

    while (start < end)
    {

        struct job *p = &jobs[c];

        switch (start[0])
        {

        case 'P':
            start = nextword(start);
            p->program = start;
            c++;

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
            return c;

        }

        start = nextword(start);

    }

    return c;

}

static unsigned int spawn(struct job *job)
{

    if (!file_walk2(FILE_L0, "/bin"))
        return 0;

    if (!(file_walk(FILE_CP, FILE_L0, job->program) || file_walk2(FILE_CP, job->program)))
        return 0;

    return call_spawn();

}

unsigned int activatenext(unsigned int startindex, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = startindex; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            channel_sendto(job->id, EVENT_MAIN);

            return i;

        }

    }

    return n;

}

void job_spawn(struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        job->id = spawn(job);

        if (!job->id)
        {

            job_sendall(jobs, n, EVENT_TERM, 0, 0);

            return;

        }

    }

}

void job_pipe(struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            channel_redirectback(job->id, EVENT_CLOSE);
            channel_redirectback(job->id, EVENT_PATH);
            channel_redirectback(job->id, EVENT_ERROR);

            if (i < n - 1)
                channel_redirecttarget(job->id, EVENT_DATA, jobs[i + 1].id);
            else
                channel_redirectback(job->id, EVENT_DATA);

        }

    }

}

void job_run(struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            unsigned int j;

            for (j = 0; j < job->noptions; j++)
            {

                struct message message;

                message_init(&message, EVENT_OPTION);
                message_putstringz(&message, job->options[j].key);
                message_putstringz(&message, job->options[j].value);
                channel_sendbufferto(job->id, EVENT_OPTION, message_datasize(&message.header), message.data.buffer);

            }

        }

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            unsigned int j;

            for (j = 0; j < job->npaths; j++)
                channel_sendstringzto(job->id, EVENT_PATH, job->paths[j]);

        }

    }

    activatenext(0, jobs, n);

}

void job_close(unsigned int id, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id == id)
        {

            job->id = 0;

            activatenext(i + 1, jobs, n);

            break;

        }

    }

}

void job_send(struct job *jobs, unsigned int n, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            channel_sendbufferto(job->id, event, count, buffer);

            break;

        }

    }

}

void job_sendall(struct job *jobs, unsigned int n, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            channel_sendbufferto(job->id, event, count, buffer);

    }

}

void job_killall(struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            call_kill(job->id);

    }

}

unsigned int job_count(struct job *jobs, unsigned int n)
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

