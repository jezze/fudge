#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

unsigned int job_parse(struct job_status *status, struct job *jobs, unsigned int n)
{

    unsigned int njobs = 0;

    memory_clear(jobs, sizeof (struct job) * n);

    while (status->start < status->end)
    {

        struct job *p = &jobs[njobs];

        switch (status->start[0])
        {

        case 'I':
            p->files[p->nfiles] = status->start + 2;
            p->nfiles++;

            break;

        case 'O':
            p->files[p->nfiles] = status->start + 2;
            p->nfiles++;

            break;

        case 'D':
            p->inputs[p->ninputs] = status->start + 2;
            p->ninputs++;

            break;

        case 'P':
            p->path = status->start + 2;

            njobs++;

            break;

        case 'E':
            status->start += ascii_length(status->start) + 1;

            return njobs;

        }

        status->start += ascii_length(status->start) + 1;

    }

    return njobs;

}

static void copyroutes(struct channel *channel, struct job *jobs, unsigned int njobs, unsigned int j)
{

    unsigned int i;

    for (i = njobs; i > j + 1; i--)
        ipc_addroute(&channel->message.header, jobs[i - 1].id);

}

void job_run(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    if (!file_walk2(FILE_L0, "/bin"))
        return;

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];

        if (!(file_walk(FILE_CP, FILE_L0, p->path) || file_walk2(FILE_CP, p->path)))
            continue;

        p->id = call_spawn(FILE_CP);

    }

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];

        if (p->nfiles || p->ninputs)
        {

            unsigned int k;

            for (k = 0; k < p->nfiles; k++)
            {

                channel_request(channel, EVENT_FILE);
                copyroutes(channel, jobs, n, i);
                channel_append(channel, ascii_length(p->files[k]) + 1, p->files[k]);
                channel_place(channel, p->id);

            }

            for (k = 0; k < p->ninputs; k++)
            {

                channel_request(channel, EVENT_DATA);
                copyroutes(channel, jobs, n, i);
                channel_append(channel, ascii_length(p->inputs[k]), p->inputs[k]);
                channel_place(channel, p->id);

            }

        }

        else
        {

            channel_request(channel, EVENT_EMPTY);
            copyroutes(channel, jobs, n, i);
            channel_place(channel, p->id);

        }

    }

    if (n)
    {

        struct job *p = &jobs[0];

        channel_request(channel, EVENT_DONE);
        copyroutes(channel, jobs, n, 0);
        channel_place(channel, p->id);

    }

}

