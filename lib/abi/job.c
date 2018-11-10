#include <fudge.h>
#include "call.h"
#include "file.h"
#include "event.h"
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

static void runjob(union event_message *imessage, union event_message *omessage, struct job *jobs, unsigned int njobs, unsigned int session)
{

    unsigned int j;
    unsigned int k;
    unsigned int x;

    for (j = 0; j < njobs; j++)
    {

        event_request(omessage, imessage, EVENT_INIT, jobs[j].id, session);

        for (x = njobs; x > j + 1; x--)
            event_route(omessage, jobs[x - 1].id);

        event_send2(omessage);

    }

    for (j = 0; j < njobs; j++)
    {

        if (jobs[j].ninputs)
        {

            for (k = 0; k < jobs[j].ninputs; k++)
            {

                event_request(omessage, imessage, EVENT_FILE, jobs[j].id, session);

                for (x = njobs; x > j + 1; x--)
                    event_route(omessage, jobs[x - 1].id);

                event_addfile(omessage, FILE_P0 + k);
                event_send2(omessage);

            }

        }

        else if (jobs[j].ndatas)
        {

            for (k = 0; k < jobs[j].ndatas; k++)
            {

                event_request(omessage, imessage, EVENT_DATA, jobs[j].id, session);

                for (x = njobs; x > j + 1; x--)
                    event_route(omessage, jobs[x - 1].id);

                event_adddata(omessage);
                event_appenddata(omessage, ascii_length(jobs[j].data[k]), jobs[j].data[k]);
                event_send2(omessage);

            }

        }

        else
        {

            event_request(omessage, imessage, EVENT_FILE, jobs[j].id, session);

            for (x = njobs; x > j + 1; x--)
                event_route(omessage, jobs[x - 1].id);

            event_addfile(omessage, 0);
            event_send2(omessage);

        }

    }

    for (j = 0; j < njobs; j++)
    {

        event_request(omessage, imessage, EVENT_STOP, jobs[j].id, session);

        for (x = njobs; x > j + 1; x--)
            event_route(omessage, jobs[x - 1].id);

        event_send2(omessage);

    }

}

void job_interpret(struct job *jobs, unsigned int njobs, union event_message *imessage, union event_message *omessage, void *buffer, unsigned int count, unsigned int session)
{

    unsigned int cjobs = clearjobs(jobs, njobs);
    char *command = buffer;
    char *end = command + count;

    if (!file_walk2(FILE_L0, "/bin"))
        return;

    for (command = buffer; command < end; command += ascii_length(command) + 1)
    {

        switch (command[0])
        {

        case 'I':
            if (!file_walk2(FILE_C0 + jobs[cjobs].ninputs, command + 2))
                return;

            jobs[cjobs].ninputs++;

            break;

        case 'O':
            if (!file_walk2(FILE_C0 + jobs[cjobs].ninputs, command + 2))
                return;

            jobs[cjobs].ninputs++;

            break;

        case 'D':
            jobs[cjobs].data[jobs[cjobs].ndatas] = command + 2;
            jobs[cjobs].ndatas++;

            break;

        case 'P':
            if (!(file_walk(FILE_CP, FILE_L0, command + 2) || file_walk2(FILE_CP, command + 2)))
                return;

            cjobs = addjob(jobs, cjobs, call_spawn());

            break;

        case 'E':
            if (!(file_walk(FILE_CP, FILE_L0, command + 2) || file_walk2(FILE_CP, command + 2)))
                return;

            cjobs = addjob(jobs, cjobs, call_spawn());

            runjob(imessage, omessage, jobs, cjobs, session);

            cjobs = clearjobs(jobs, cjobs);

            break;

        }

    }

}

