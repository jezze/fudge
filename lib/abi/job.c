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

        case 'F':
            p->files[p->nfiles] = status->start + 2;
            p->nfiles++;

            break;

        case 'R':
            p->redirects[p->nredirects] = status->start + 2;
            p->nredirects++;

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
        struct event_redirect redirect;

        if (i < n - 1)
        {

            redirect.mode = 1;
            redirect.id = jobs[i + 1].id;

        }

        else
        {

            redirect.mode = 2;
            redirect.id = 0;

        }

        redirect.type = EVENT_DATA;

        channel_request(channel, EVENT_REDIRECT);
        channel_append(channel, sizeof (struct event_redirect), &redirect);
        channel_place(channel, p->id);

    }

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];
        unsigned int k;

        for (k = 0; k < p->nfiles; k++)
        {

            channel_request(channel, EVENT_FILE);
            channel_appendstring2(channel, p->files[k]);
            channel_place(channel, p->id);

        }

        for (k = 0; k < p->ninputs; k++)
        {

            channel_request(channel, EVENT_DATA);
            channel_appendstring(channel, p->inputs[k]);
            channel_place(channel, p->id);

        }

    }

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];

        channel_request(channel, EVENT_MAIN);
        channel_place(channel, p->id);

    }

}

