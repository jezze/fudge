#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"
#include "job.h"

static unsigned int find(struct job *job, unsigned int target)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        if (job->commands[i].target == target)
            return i;

    }

    return job->ncommands;

}

unsigned int job_spawn(struct job *job, unsigned int ichannel, unsigned int notify, char *bindir)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        command->target = fs_spawn_relative(ichannel, notify, command->program, bindir);

        if (!command->target)
            command->target = fs_spawn(ichannel, notify, command->program);

        if (!command->target)
        {

            cstring_write_fmt1(job->error, JOB_ERRORSIZE, 0, "Command not found: %s\\0", command->program);

            return 0;

        }

    }

    return job->ncommands;

}

void job_run(struct job *job, unsigned int ichannel, char *pwd)
{

    unsigned int i;

    for (i = job->ncommands; i > 0; i--)
    {

        struct job_command *command = &job->commands[i - 1];
        struct event_pipe pipe;
        char options[MESSAGE_SIZE];
        unsigned int count;
        unsigned int j;

        pipe.prev = (i > 1) ? job->commands[i - 2].target : 0;
        pipe.next = (i < job->ncommands) ? job->commands[i].target : 0;
        count = cstring_write_fmt1(options, MESSAGE_SIZE, 0, "pwd=%s", pwd);

        for (j = 0; j < command->noptions; j++)
            count += cstring_write_fmt2(options, MESSAGE_SIZE, count, "&%s=%s", command->keys[j], command->values[j]);

        count += cstring_write_fmt0(options, MESSAGE_SIZE, count, "\n");

        channel_send(ichannel, command->target, EVENT_PIPE, sizeof (struct event_pipe), &pipe);
        channel_send(ichannel, command->target, EVENT_OPTION, count, options);
        channel_send(ichannel, command->target, EVENT_MAIN, 0, 0);

        count = 0;

        for (j = 0; j < command->npaths; j++)
        {

            char *path = command->paths[j];
            char *prefix = (fs_auth(path)) ? "" : pwd;

            if (count + cstring_length(prefix) + cstring_length(path) + 1 > MESSAGE_SIZE)
            {

                channel_send(ichannel, command->target, EVENT_PATH, count, options);

                count = 0;

            }

            count += cstring_write_fmt2(options, MESSAGE_SIZE, count, "%s%s\\0", prefix, path);

        }

        if (count)
            channel_send(ichannel, command->target, EVENT_PATH, count, options);

    }

}

void job_abort(struct job *job, unsigned int ichannel)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        if (command->target)
            channel_send(ichannel, command->target, EVENT_TERM, 0, 0);

    }

}

void job_detach(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
        job->commands[i].target = 0;

}

unsigned int job_exist(struct job *job, unsigned int target)
{

    return (find(job, target) < job->ncommands) ? target : 0;

}

unsigned int job_exit(struct job *job, unsigned int ichannel, unsigned int target, unsigned int status)
{

    unsigned int index = find(job, target);

    if (index < job->ncommands)
    {

        if (status != EXIT_STATUS_NORMAL && index + 1 < job->ncommands && job->commands[index + 1].target)
            channel_send(ichannel, job->commands[index + 1].target, EVENT_TERM, 0, 0);

        job->commands[index].target = 0;

        return 1;

    }

    return 0;

}

void job_kill(struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        if (command->target)
            call_kill(command->target);

    }

}

void job_sendfirst(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        if (command->target)
        {

            channel_send(ichannel, command->target, event, count, buffer);

            break;

        }

    }

}

void job_sendall(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        if (command->target)
            channel_send(ichannel, command->target, event, count, buffer);

    }

}

unsigned int job_count(struct job *job)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        if (job->commands[i].target)
            count++;

    }

    return count;

}

