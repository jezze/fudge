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
#define TOKEN_BACKGROUND                6

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
    case '&':
    case '\n':
        return 1;

    }

    return 0;

}

static unsigned int isreserved(char *s)
{

    for (; *s; s++)
    {

        if (*s == '&' || *s == '=')
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

    case '&':
        parser->offset++;

        return TOKEN_BACKGROUND;

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

    if (job->nstrings >= JOB_STRINGSSIZE)
    {

        seterror(job, "Syntax error: Line too long", 0);

        return TOKEN_ERROR;

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
                if (isreserved(word) || isreserved(value))
                    return seterror(job, "Syntax error: Unexpected & or = in -%s", word);

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

        case TOKEN_BACKGROUND:
            if (!command)
                return seterror(job, "Syntax error: Expected command before &", 0);

            job->background = 1;

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
            return seterror(job, "Command not found: %s", command->program);

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

        for (j = 0; j < command->npaths; j++)
        {

            char *path = command->paths[j];

            if (fs_auth(path))
                channel_send_fmt1(ichannel, command->target, EVENT_PATH, "%s\\0", path);
            else
                channel_send_fmt2(ichannel, command->target, EVENT_PATH, "%s%s\\0", pwd, path);

        }

    }

    channel_send(ichannel, job->commands[0].target, EVENT_TERM, 0, 0);

    if (job->background)
    {

        for (i = 0; i < job->ncommands; i++)
            job->commands[i].target = 0;

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

