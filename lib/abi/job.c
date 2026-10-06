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

static unsigned int linearprev(struct job *job, unsigned int index)
{

    while (index > 0)
    {

        index--;

        if (!job->commands[index].side)
            return job->commands[index].target;

    }

    return 0;

}

static unsigned int linearnext(struct job *job, unsigned int index)
{

    for (index++; index < job->ncommands; index++)
    {

        if (!job->commands[index].side)
            return job->commands[index].target;

    }

    return 0;

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

        pipe.prev = (command->side) ? 0 : linearprev(job, i - 1);
        pipe.next = (command->side) ? 0 : linearnext(job, i - 1);
        pipe.nroutes = command->nroutes;

        for (j = 0; j < command->nroutes; j++)
        {

            struct job_route *route = &command->routes[j];

            pipe.routes[j].event = route->event;
            pipe.routes[j].to = route->to;
            pipe.routes[j].target = (route->stage) ? job->commands[route->stage - 1].target : 0;

        }
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
            char full[1024];
            char canonical[1024];
            unsigned int length;

            cstring_write_fmt2(full, 1024, 0, "%s%s\\0", (buffer_eachbyte(path, cstring_length(path), ':', 0)) ? "" : pwd, path);

            length = fs_canonical(canonical, 1024, full);

            if (count + length + 1 > MESSAGE_SIZE)
            {

                channel_send(ichannel, command->target, EVENT_PATH, count, options);

                count = 0;

            }

            count += buffer_write(options, MESSAGE_SIZE, canonical, length + 1, count);

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

        if (status != EXIT_STATUS_NORMAL)
        {

            unsigned int next = linearnext(job, index);
            unsigned int i;

            if (next)
                channel_send(ichannel, next, EVENT_TERM, 0, 0);

            for (i = 0; i < job->ncommands; i++)
            {

                if (job->commands[i].side == index + 1 && job->commands[i].target)
                    channel_send(ichannel, job->commands[i].target, EVENT_TERM, 0, 0);

            }

        }

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

