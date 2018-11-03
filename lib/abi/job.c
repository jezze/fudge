#include <fudge.h>
#include "call.h"
#include "file.h"
#include "event.h"
#include "job.h"

static unsigned int addjob(struct job *job, unsigned int count, unsigned int id)
{

    if (id)
    {

        job[count].id = id;

        return count + 1;

    }

    return count;

}

static unsigned int clearjobs(struct job *job, unsigned int count)
{

    memory_clear(job, sizeof (struct job) * count);

    return 0;

}

static void runjob(struct event_header *iheader, struct event_header *oheader, struct job *job, unsigned int count, unsigned int session)
{

    unsigned int j;
    unsigned int k;
    unsigned int x;

    for (j = 0; j < count; j++)
    {

        event_requestinit(oheader, iheader, job[j].id, session);

        for (x = count; x > j + 1; x--)
            event_route(oheader, job[x - 1].id);

        event_send(oheader);

    }

    for (j = 0; j < count; j++)
    {

        if (job[j].ninputs)
        {

            for (k = 0; k < job[j].ninputs; k++)
            {

                event_requestfile(oheader, iheader, job[j].id, session, FILE_P0 + k);

                for (x = count; x > j + 1; x--)
                    event_route(oheader, job[x - 1].id);

                event_send(oheader);

            }

        }

        else if (job[j].ndatas)
        {

            for (k = 0; k < job[j].ndatas; k++)
            {

                event_requestdata(oheader, iheader, job[j].id, session);

                for (x = count; x > j + 1; x--)
                    event_route(oheader, job[x - 1].id);

                event_appenddata(oheader, ascii_length(job[j].data[k]), job[j].data[k]);
                event_send(oheader);

            }

        }

        else
        {

            event_requestfile(oheader, iheader, job[j].id, session, 0);

            for (x = count; x > j + 1; x--)
                event_route(oheader, job[x - 1].id);

            event_send(oheader);

        }

    }

    for (j = 0; j < count; j++)
    {

        event_requeststop(oheader, iheader, job[j].id, session);

        for (x = count; x > j + 1; x--)
            event_route(oheader, job[x - 1].id);

        event_send(oheader);

    }

}

void job_run(struct event_header *iheader, struct event_header *oheader, void *buffer, unsigned int count, unsigned int session)
{

    char *command = buffer;
    char *end = command + count;
    struct job job[32];
    unsigned int njobs = clearjobs(job, 32);

    if (!file_walk2(FILE_L0, "/bin"))
        return;

    for (command = buffer; command < end; command += ascii_length(command) + 1)
    {

        switch (command[0])
        {

        case 'I':
            if (!file_walk2(FILE_C0 + job[njobs].ninputs, command + 2))
                return;

            job[njobs].ninputs++;

            break;

        case 'O':
            if (!file_walk2(FILE_C0 + job[njobs].ninputs, command + 2))
                return;

            job[njobs].ninputs++;

            break;

        case 'D':
            job[njobs].data[job[njobs].ndatas] = command + 2;
            job[njobs].ndatas++;

            break;

        case 'P':
            if (!(file_walk(FILE_CP, FILE_L0, command + 2) || file_walk2(FILE_CP, command + 2)))
                return;

            njobs = addjob(job, njobs, call_spawn());

            break;

        case 'E':
            if (!(file_walk(FILE_CP, FILE_L0, command + 2) || file_walk2(FILE_CP, command + 2)))
                return;

            njobs = addjob(job, njobs, call_spawn());

            runjob(iheader, oheader, job, njobs, session);

            njobs = clearjobs(job, njobs);

            break;

        }

    }

}

