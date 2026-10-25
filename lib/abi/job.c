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

static struct event_route *findroute(struct event_pipe *pipe, unsigned int event)
{

    unsigned int i;

    for (i = 0; i < pipe->nroutes; i++)
    {

        if (pipe->routes[i].event == event)
            return &pipe->routes[i];

    }

    return 0;

}

static void addroute(struct event_pipe *pipe, unsigned int event, unsigned int to, unsigned int target)
{

    if (!findroute(pipe, event))
    {

        struct event_route *route = &pipe->routes[pipe->nroutes++];

        route->event = event;
        route->to = to;
        route->target = target;

    }

}

static void addterm(struct event_pipe *pipe, unsigned int target, unsigned int to)
{

    struct event_route *term;
    unsigned int i;

    for (i = 0; i < pipe->nterms; i++)
    {

        if (pipe->terms[i].target == target)
            return;

    }

    term = &pipe->terms[pipe->nterms++];
    term->event = EVENT_TERM;
    term->to = to;
    term->target = target;

}

static void setpipe(struct job *job, unsigned int index, struct event_pipe *pipe)
{

    struct job_command *command = &job->commands[index];
    unsigned int next = (command->side) ? 0 : linearnext(job, index);
    struct event_route *term;
    unsigned int i;

    buffer_clear(pipe, sizeof (struct event_pipe));

    pipe->prev = (command->side) ? job->commands[command->side - 1].target : linearprev(job, index);

    for (i = 0; i < command->nroutes; i++)
    {

        struct job_route *route = &command->routes[i];

        addroute(pipe, route->event, route->to, (route->stage) ? job->commands[route->stage - 1].target : ((route->to == EVENT_ERROR) ? 0 : next));

    }

    addroute(pipe, EVENT_DATA, EVENT_DATA, next);
    addroute(pipe, EVENT_ERROR, EVENT_ERROR, 0);

    term = findroute(pipe, EVENT_TERM);

    if (term)
        addterm(pipe, term->target, term->to);
    else
        addterm(pipe, next, EVENT_TERM);

    for (i = 0; i < command->nroutes; i++)
    {

        if (command->routes[i].stage)
            addterm(pipe, job->commands[command->routes[i].stage - 1].target, EVENT_TERM);

    }

    if (pipe->terms[0].target || pipe->terms[0].to != EVENT_TERM)
    {

        term = &pipe->terms[pipe->nterms++];
        term->event = EVENT_TERM;
        term->to = EVENT_TERM;
        term->target = 0;

    }

}

unsigned int job_spawn(struct job *job, unsigned int ichannel, char *bindir)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        command->target = fs_spawn_relative(ichannel, command->program, bindir);

        if (!command->target)
            command->target = fs_spawn(ichannel, command->program);

        if (!command->target)
        {

            cstring_write_fmt(job->error, JOB_ERRORSIZE, 0, "Command not found: %s\\0", command->program);

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

        setpipe(job, i - 1, &pipe);

        count = cstring_write_fmt(options, MESSAGE_SIZE, 0, "pwd\\0%s\\0", pwd);

        for (j = 0; j < command->noptions; j++)
            count += cstring_write_fmt(options, MESSAGE_SIZE, count, "%s\\0%s\\0", command->keys[j], command->values[j]);

        channel_send(ichannel, command->target, EVENT_OPTION, count, options);
        channel_send(ichannel, command->target, EVENT_MAIN, sizeof (struct event_pipe), &pipe);

        for (j = 0; j < command->npaths; j++)
        {

            char absolute[1024];
            unsigned int length = fs_absolute(absolute, 1024, pwd, command->paths[j]);

            channel_send(ichannel, command->target, EVENT_PATH, length + 1, absolute);

        }

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

unsigned int job_exit(struct job *job, unsigned int target)
{

    unsigned int index = find(job, target);

    if (index < job->ncommands)
    {

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

        if (command->target && !call_kill(command->target))
            command->target = 0;

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

