#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

static unsigned int addjob(struct job *jobs, unsigned int njobs, unsigned int id)
{

    if (id)
    {

        jobs[njobs].id = id;

        return njobs + 1;

    }

    return njobs;

}

static unsigned int clearjobs(struct job *jobs, unsigned int njobs)
{

    memory_clear(jobs, sizeof (struct job) * njobs);

    return 0;

}

static void runjob(struct channel *channel, struct job *jobs, unsigned int njobs, unsigned int session)
{

    unsigned int j;
    unsigned int k;
    unsigned int x;

    for (j = 0; j < njobs; j++)
    {

        if (jobs[j].ninputs || jobs[j].ndatas)
        {

            for (k = 0; k < jobs[j].ninputs; k++)
            {

                channel_request(channel, EVENT_FILE, session);

                for (x = njobs; x > j + 1; x--)
                    event_addroute(&channel->o.header, jobs[x - 1].id, session);

                event_addfile(&channel->o, FILE_P0 + k);
                channel_place(jobs[j].id, &channel->o.header);

            }

            for (k = 0; k < jobs[j].ndatas; k++)
            {

                channel_request(channel, EVENT_DATA, session);

                for (x = njobs; x > j + 1; x--)
                    event_addroute(&channel->o.header, jobs[x - 1].id, session);

                event_append(&channel->o.header, ascii_length(jobs[j].data[k]), jobs[j].data[k]);
                channel_place(jobs[j].id, &channel->o.header);

            }

        }

        else
        {

            channel_request(channel, EVENT_EMPTY, session);

            for (x = njobs; x > j + 1; x--)
                event_addroute(&channel->o.header, jobs[x - 1].id, session);

            channel_place(jobs[j].id, &channel->o.header);

        }

        channel_request(channel, EVENT_STOP, session);

        for (x = njobs; x > j + 1; x--)
            event_addroute(&channel->o.header, jobs[x - 1].id, session);

        channel_place(jobs[j].id, &channel->o.header);

    }

}

void job_interpret(struct job *jobs, unsigned int njobs, struct channel *channel, void *buffer, unsigned int count, unsigned int session)
{

    unsigned int cjobs = clearjobs(jobs, njobs);
    char *start = buffer;
    char *end = start + count;

    if (!file_walk2(FILE_L0, "/bin"))
        return;

    while (start < end)
    {

        struct job *job = &jobs[cjobs];

        switch (start[0])
        {

        case 'I':
            if (!file_walk2(FILE_C0 + job->ninputs, start + 2))
                return;

            job->ninputs++;

            break;

        case 'O':
            if (!file_walk2(FILE_C0 + job->ninputs, start + 2))
                return;

            job->ninputs++;

            break;

        case 'D':
            job->data[job->ndatas] = start + 2;
            job->ndatas++;

            break;

        case 'P':
            if (!(file_walk(FILE_CP, FILE_L0, start + 2) || file_walk2(FILE_CP, start + 2)))
                return;

            cjobs = addjob(jobs, cjobs, call_spawn());

            break;

        case 'E':
            if (!(file_walk(FILE_CP, FILE_L0, start + 2) || file_walk2(FILE_CP, start + 2)))
                return;

            cjobs = addjob(jobs, cjobs, call_spawn());

            runjob(channel, jobs, cjobs, session);

            cjobs = clearjobs(jobs, cjobs);

            break;

        }

        start += ascii_length(start) + 1;

    }

}

