#include <fudge.h>
#include <abi.h>

#define BINPATH                         "initrd:bin"
#define SCRIPTSIZE                      0x4000

static struct job job;
static char script[SCRIPTSIZE];
static unsigned int scriptcount;
static unsigned int output;
static unsigned int running;
static unsigned int interrupts;
static unsigned int inputopen;

#define TOKEN_END                       1
#define TOKEN_WORD                      2
#define TOKEN_OPTION                    3
#define TOKEN_PIPE                      4
#define TOKEN_ERROR                     5
#define TOKEN_BACKGROUND                6
#define TOKEN_INPUT                     7
#define TOKEN_OUTPUT                    8
#define TOKEN_APPEND                    9

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
    case '<':
    case '>':
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

    case '<':
        parser->offset++;

        return TOKEN_INPUT;

    case '>':
        parser->offset++;

        if (parser->offset < parser->count && parser->data[parser->offset] == '>')
        {

            parser->offset++;

            return TOKEN_APPEND;

        }

        return TOKEN_OUTPUT;

    case '#':
        while (parser->offset < parser->count && parser->data[parser->offset] != '\n')
            parser->offset++;

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

static unsigned int readpath(struct job *job, struct parser *parser, char **word, char *redirect)
{

    switch (readtoken(job, parser, word))
    {

    case TOKEN_WORD:
        return 1;

    case TOKEN_ERROR:
        return 0;

    }

    return seterror(job, "Syntax error: Expected path after %s", redirect);

}

static struct job_command *addinput(struct job *job, struct job_command *command, char *path)
{

    unsigned int i;

    for (i = job->ncommands; i > 0; i--)
        job->commands[i] = job->commands[i - 1];

    job->ncommands++;

    buffer_clear(&job->commands[0], sizeof (struct job_command));

    job->commands[0].program = "echo";
    job->commands[0].paths[0] = path;
    job->commands[0].npaths = 1;

    return command + 1;

}

static void addoutput(struct job *job, char *path, char *mode)
{

    struct job_command *command = &job->commands[job->ncommands++];

    command->program = "write";
    command->paths[0] = path;
    command->npaths = 1;
    command->keys[0] = mode;
    command->values[0] = "1";
    command->noptions = 1;

}

static unsigned int parse(struct job *job, struct parser *parser)
{

    struct job_command *command = 0;
    unsigned int redirected = 0;

    for (;;)
    {

        char *word = 0;
        char *value = 0;
        unsigned int token = readtoken(job, parser, &word);

        switch (token)
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

            if (redirected)
                return seterror(job, "Syntax error: Unexpected | after > or >>", 0);

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

        case TOKEN_INPUT:
            if (!command)
                return seterror(job, "Syntax error: Expected command before <", 0);

            if (command != &job->commands[0])
                return seterror(job, "Syntax error: < only works on the first command", 0);

            if (job->ncommands >= JOB_COMMANDS)
                return seterror(job, "Syntax error: Too many commands in pipeline", 0);

            if (!readpath(job, parser, &word, "<"))
                return 0;

            command = addinput(job, command, word);

            break;

        case TOKEN_OUTPUT:
        case TOKEN_APPEND:
            if (!command)
                return seterror(job, "Syntax error: Expected command before > or >>", 0);

            if (redirected)
                return seterror(job, "Syntax error: Only one > or >> per command line", 0);

            if (job->ncommands >= JOB_COMMANDS)
                return seterror(job, "Syntax error: Too many commands in pipeline", 0);

            if (!readpath(job, parser, &word, (token == TOKEN_APPEND) ? ">>" : ">"))
                return 0;

            addoutput(job, word, (token == TOKEN_APPEND) ? "append" : "create");

            redirected = 1;

            break;

        case TOKEN_ERROR:
            return 0;

        }

    }

}

static unsigned int parseline(struct job *job, char *data, unsigned int count, unsigned int *offset)
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


static void run(char *pwd)
{

    unsigned int offset = 0;

    while (offset < scriptcount && !interrupts)
    {

        if (!parseline(&job, script, scriptcount, &offset))
        {

            channel_send_fmt1(0, output, EVENT_ERROR, "%s\n", job.error);

            return;

        }

        if (!job.ncommands)
            continue;

        if (!job_spawn(&job, 1, 0, BINPATH))
        {

            channel_send_fmt1(0, output, EVENT_ERROR, "%s\n", job.error);
            job_abort(&job, 0);

            continue;

        }

        job_run(&job, 0, pwd);

        if (!inputopen || job.background)
            job_sendfirst(&job, 0, EVENT_TERM, 0, 0);

        if (job.background)
            job_detach(&job);

        while (job_count(&job))
        {

            if (!channel_process(0))
            {

                job_sendall(&job, 0, EVENT_INTERRUPT, 0, 0);

                return;

            }

        }

    }

}

static void append(void *data, unsigned int count)
{

    scriptcount += buffer_write(script, SCRIPTSIZE - 1, data, count, scriptcount);

}

static void onconsoledata(struct message *message)
{

    char *data = message->data;

    if (message->length == 1 && data[0] == 0x03)
    {

        if (interrupts++)
            job_kill(&job);
        else
            job_sendall(&job, 0, EVENT_INTERRUPT, 0, 0);

        return;

    }

    job_sendfirst(&job, 0, EVENT_CONSOLEDATA, message->length, message->data);

}

static void ondata(struct message *message)
{

    if (job_exist(&job, message->source))
        channel_send(0, output, EVENT_DATA, message->length, message->data);
    else if (running)
        job_sendfirst(&job, 0, EVENT_DATA, message->length, message->data);
    else
        append(message->data, message->length);

}

static void onerror(struct message *message)
{

    if (job_exist(&job, message->source))
        channel_send(0, output, EVENT_ERROR, message->length, message->data);

}

static void onexit(struct message *message)
{

    struct event_exit *exit = message->data;

    job_exit(&job, 0, message->source, exit->status);

}

static void onmain(struct message *message)
{

    output = message->source;

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);
    unsigned int id = (target) ? fs_walk(1, target, 0, message->data) : 0;
    char buffer[0x800];
    unsigned int offset = 0;
    unsigned int count;

    if (!id)
    {

        channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        return;

    }

    while ((count = fs_read(1, target, id, buffer, 0x800, offset)))
    {

        append(buffer, count);

        offset += count;

    }

    append("\n", 1);

}

static void onterm(struct message *message)
{

    if (running)
    {

        inputopen = 0;

        job_sendfirst(&job, 0, EVENT_TERM, 0, 0);

        return;

    }

    running = 1;
    inputopen = option_getdecimal("input");

    run(option_getstring("pwd"));

    if (option_getdecimal("export"))
        channel_send_fmt1(0, output, EVENT_OPTION, "pwd=%s\n", option_getstring("pwd"));

}

void init(void)
{

    option_add("export", "0");
    option_add("input", "0");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

