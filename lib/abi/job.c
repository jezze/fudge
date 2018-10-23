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

        event_requestinit(oheader, iheader, job[j].id);
        event_send(oheader);

    }

    for (j = 0; j < count; j++)
    {

        if (job[j].ninputs)
        {

            for (k = 0; k < job[j].ninputs; k++)
            {

                event_requestdatafile(oheader, iheader, job[j].id, session, FILE_P0 + k);

                for (x = count; x > j + 1; x--)
                    event_route(oheader, job[x - 1].id);

                event_send(oheader);

            }

        }

        else
        {

            event_requestdatafile(oheader, iheader, job[j].id, session, 0);

            for (x = count; x > j + 1; x--)
                event_route(oheader, job[x - 1].id);

            event_send(oheader);

        }

    }

    event_requestdatastop(oheader, iheader, job[0].id, session);

    for (x = count; x > 1; x--)
        event_route(oheader, job[x - 1].id);

    event_send(oheader);

}

unsigned int job_runcmd(struct event_header *iheader, struct event_header *oheader, char *command, char *data, unsigned int count, unsigned int session)
{

    unsigned int id;

    if (!file_walk(FILE_CP, command))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_requestinit(oheader, iheader, id);
        event_send(oheader);
        event_requestdatapipe(oheader, iheader, id, session);
        event_appenddata(oheader, count, data);
        event_send(oheader);
        event_requestdatastop(oheader, iheader, id, session);
        event_send(oheader);

    }

    return id;

}

void job_run(struct event_header *iheader, struct event_header *oheader, void *buffer, unsigned int count)
{

    char *command = buffer;
    char *end = command + count;
    struct job job[32];
    unsigned int njobs = clearjobs(job, 32);

    if (!file_walk(FILE_L0, "/bin"))
        return;

    for (command = buffer; command < end; command += ascii_length(command) + 1)
    {

        switch (command[0])
        {

        case 'I':
            if (!file_walk(FILE_C0 + job[njobs].ninputs, command + 1))
                return;

            job[njobs].ninputs++;

            break;

        case 'O':
            if (!file_walk(FILE_C0 + job[njobs].ninputs, command + 1))
                return;

            job[njobs].ninputs++;

            break;

        case 'P':
            if (!(file_walkfrom(FILE_CP, FILE_L0, command + 1) || file_walk(FILE_CP, command + 1)))
                return;

            njobs = addjob(job, njobs, call_spawn());

            break;

        case 'E':
            if (!(file_walkfrom(FILE_CP, FILE_L0, command + 1) || file_walk(FILE_CP, command + 1)))
                return;

            njobs = addjob(job, njobs, call_spawn());

            runjob(iheader, oheader, job, njobs, 0);

            njobs = clearjobs(job, njobs);

            break;

        }

    }

}

