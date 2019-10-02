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

static void copyroutes(struct channel *channel, struct job *jobs, unsigned int njobs, unsigned int session, unsigned int j)
{

    unsigned int i;

    for (i = njobs; i > j + 1; i--)
        ipc_addroute(&channel->message.header, jobs[i - 1].id, session);

}

static void runjob(struct channel *channel, struct job *jobs, unsigned int njobs, unsigned int session)
{

    unsigned int i;

    for (i = 0; i < njobs; i++)
    {

        struct job *job = &jobs[i];

        channel_request2(channel, EVENT_OPEN, session);
        copyroutes(channel, jobs, njobs, session, i);
        channel_place(channel, job->id);

        if (job->ninputs || job->ndatas)
        {

            unsigned int k;

            for (k = 0; k < job->ninputs; k++)
            {

                struct event_file file;

                file.descriptor = FILE_P0 + k;

                channel_request2(channel, EVENT_FILE, session);
                copyroutes(channel, jobs, njobs, session, i);
                channel_append(channel, sizeof (struct event_file), &file);
                channel_place(channel, job->id);

            }

            for (k = 0; k < job->ndatas; k++)
            {

                channel_request2(channel, EVENT_DATA, session);
                copyroutes(channel, jobs, njobs, session, i);
                channel_append(channel, ascii_length(job->data[k]), job->data[k]);
                channel_place(channel, job->id);

            }

        }

        else
        {

            channel_request2(channel, EVENT_EMPTY, session);
            copyroutes(channel, jobs, njobs, session, i);
            channel_place(channel, job->id);

        }

    }

    if (njobs > 0)
    {

        struct job *job = &jobs[0];

        channel_request2(channel, EVENT_CLOSE, session);
        copyroutes(channel, jobs, njobs, session, 0);
        channel_place(channel, job->id);

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

