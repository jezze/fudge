#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"
#include "job.h"

#define TOKEN_END                       1
#define TOKEN_WORD                      2
#define TOKEN_OPTION                    3
#define TOKEN_PIPE                      4
#define TOKEN_ERROR                     5

struct parser
{

    char *data;
    unsigned int count;
    unsigned int offset;

};

static unsigned int isblankchar(char c)
{

    return (c == ' ' || c == '\t');

}

static unsigned int isspecialchar(char c)
{

    switch (c)
    {

    case ' ':
    case '\t':
    case '|':
    case ';':
    case '\n':
        return 1;

    }

    return 0;

}

static unsigned int seterror(struct job *job, char *fmt, char *arg)
{

    unsigned int count = cstring_write_fmt1(job->error, JOB_ERRORSIZE - 1, 0, fmt, arg);

    job->error[count] = '\0';

    return 0;

}

static void addchar(struct job *job, char c)
{

    if (job->nstrings < JOB_STRINGSSIZE)
        job->strings[job->nstrings++] = c;

}

static unsigned int readtoken(struct job *job, struct parser *parser, char **word)
{

    unsigned int type = TOKEN_WORD;

    while (parser->offset < parser->count && isblankchar(parser->data[parser->offset]))
        parser->offset++;

    if (parser->offset >= parser->count)
        return TOKEN_END;

    switch (parser->data[parser->offset])
    {

    case '|':
        parser->offset++;

        return TOKEN_PIPE;

    case ';':
    case '\n':
        parser->offset++;

        return TOKEN_END;

    case '-':
        parser->offset++;

        type = TOKEN_OPTION;

        break;

    }

    *word = job->strings + job->nstrings;

    while (parser->offset < parser->count && !isspecialchar(parser->data[parser->offset]))
    {

        char c = parser->data[parser->offset++];

        if (c == '"' || c == '\'')
        {

            while (parser->offset < parser->count && parser->data[parser->offset] != c)
                addchar(job, parser->data[parser->offset++]);

            if (parser->offset >= parser->count)
            {

                seterror(job, "Syntax error: Unterminated quote", 0);

                return TOKEN_ERROR;

            }

            parser->offset++;

        }

        else
        {

            addchar(job, c);

        }

    }

    addchar(job, '\0');

    if (type == TOKEN_OPTION && !cstring_length(*word))
    {

        seterror(job, "Syntax error: Expected option name after -", 0);

        return TOKEN_ERROR;

    }

    return type;

}

static unsigned int parse(struct job *job, struct parser *parser)
{

    struct job_command *command = 0;

    for (;;)
    {

        char *word = 0;
        char *value = 0;

        switch (readtoken(job, parser, &word))
        {

        case TOKEN_WORD:
            if (command)
            {

                if (command->npaths >= JOB_PATHS)
                    return seterror(job, "Syntax error: Too many arguments", 0);

                command->paths[command->npaths++] = word;

            }

            else
            {

                if (job->ncommands >= JOB_COMMANDS)
                    return seterror(job, "Syntax error: Too many commands in pipeline", 0);

                command = &job->commands[job->ncommands++];
                command->program = word;

            }

            break;

        case TOKEN_OPTION:
            if (!command)
                return seterror(job, "Syntax error: Expected command before option", 0);

            if (command->noptions >= JOB_OPTIONS)
                return seterror(job, "Syntax error: Too many options", 0);

            switch (readtoken(job, parser, &value))
            {

            case TOKEN_WORD:
                command->keys[command->noptions] = word;
                command->values[command->noptions] = value;
                command->noptions++;

                break;

            case TOKEN_ERROR:
                return 0;

            default:
                return seterror(job, "Syntax error: Expected value after -%s", word);

            }

            break;

        case TOKEN_PIPE:
            if (!command)
                return seterror(job, "Syntax error: Expected command before |", 0);

            command = 0;

            break;

        case TOKEN_END:
            if (job->ncommands && !command)
                return seterror(job, "Syntax error: Expected command after |", 0);

            return 1;

        case TOKEN_ERROR:
            return 0;

        }

    }

}

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

static void activate(struct job *job, unsigned int ichannel, unsigned int start)
{

    unsigned int i;

    for (i = start; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        if (command->target && !command->finished)
        {

            if (!command->terminated)
            {

                channel_send(ichannel, command->target, EVENT_TERM, 0, 0);

                command->terminated = 1;

            }

            break;

        }

    }

}

static void finish(struct job *job, unsigned int ichannel, unsigned int index)
{

    struct job_command *command = &job->commands[index];

    if (!command->finished)
    {

        command->finished = 1;

        activate(job, ichannel, index + 1);

    }

}

unsigned int job_parse(struct job *job, char *data, unsigned int count, unsigned int *offset)
{

    struct parser parser;
    unsigned int status;

    buffer_clear(job, sizeof (struct job));

    parser.data = data;
    parser.count = count;
    parser.offset = *offset;

    status = parse(job, &parser);

    *offset = parser.offset;

    return status;

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
            return seterror(job, "Command not found: %s", command->program);

    }

    return job->ncommands;

}

void job_run(struct job *job, unsigned int ichannel, char *pwd)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];
        unsigned int j;

        channel_send_fmt1(ichannel, command->target, EVENT_OPTION, "pwd=%s\n", pwd);

        for (j = 0; j < command->noptions; j++)
            channel_send_fmt2(ichannel, command->target, EVENT_OPTION, "%s=%s\n", command->keys[j], command->values[j]);

    }

    for (i = 0; i < job->ncommands; i++)
        channel_send(ichannel, job->commands[i].target, EVENT_MAIN, 0, 0);

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];
        unsigned int j;

        for (j = 0; j < command->npaths; j++)
        {

            char *path = command->paths[j];

            if (fs_auth(path))
                channel_send_fmt1(ichannel, command->target, EVENT_PATH, "%s\\0", path);
            else
                channel_send_fmt2(ichannel, command->target, EVENT_PATH, "%s%s\\0", pwd, path);

        }

    }

    activate(job, ichannel, 0);

}

void job_abort(struct job *job, unsigned int ichannel)
{

    unsigned int i;

    for (i = 0; i < job->ncommands; i++)
    {

        struct job_command *command = &job->commands[i];

        if (command->target && !command->terminated)
            channel_send(ichannel, command->target, EVENT_TERM, 0, 0);

        command->finished = 1;
        command->terminated = 1;

    }

}

unsigned int job_exist(struct job *job, unsigned int target)
{

    return (find(job, target) < job->ncommands) ? target : 0;

}

unsigned int job_pipe(struct job *job, unsigned int ichannel, struct message *message)
{

    unsigned int index = find(job, message->source);

    if (index + 1 < job->ncommands)
    {

        struct job_command *next = &job->commands[index + 1];

        if (next->target)
            channel_send(ichannel, next->target, message->event, message->length, message->data);

        return 1;

    }

    return 0;

}

unsigned int job_close(struct job *job, unsigned int ichannel, unsigned int target)
{

    unsigned int index = find(job, target);

    if (index < job->ncommands)
    {

        finish(job, ichannel, index);

        return 1;

    }

    return 0;

}

unsigned int job_exit(struct job *job, unsigned int ichannel, unsigned int target)
{

    unsigned int index = find(job, target);

    if (index < job->ncommands)
    {

        finish(job, ichannel, index);

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

        if (command->target && !command->finished)
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

