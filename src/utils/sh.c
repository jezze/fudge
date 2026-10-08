#include <fudge.h>
#include <abi.h>

#define BINPATH                         "initrd:bin"
#define SCRIPTSIZE                      0x4000
#define TOKEN_END                       1
#define TOKEN_WORD                      2
#define TOKEN_OPTION                    3
#define TOKEN_PIPE                      4
#define TOKEN_ERROR                     5
#define TOKEN_BACKGROUND                6
#define TOKEN_INPUT                     7
#define TOKEN_OUTPUT                    8
#define TOKEN_APPEND                    9
#define TOKEN_DUP                       10

static struct job job;
static char script[SCRIPTSIZE];
static unsigned int scriptcount;
static unsigned int output;
static unsigned int running;
static unsigned int interrupts;
static unsigned int inputopen;

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

static unsigned int seterror(struct job *job, char *fmt, char *arg)
{

    unsigned int count = cstring_write_fmt(job->error, JOB_ERRORSIZE - 1, 0, fmt, arg);

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

        if (parser->offset < parser->count && parser->data[parser->offset] == '&')
        {

            parser->offset++;

            return TOKEN_DUP;

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

static struct {char *name; unsigned int event;} events[] = {
    {"main", EVENT_MAIN},
    {"term", EVENT_TERM},
    {"interrupt", EVENT_INTERRUPT},
    {"option", EVENT_OPTION},
    {"path", EVENT_PATH},
    {"data", EVENT_DATA},
    {"error", EVENT_ERROR},
    {"status", EVENT_STATUS},
    {"link", EVENT_LINK},
    {"unlink", EVENT_UNLINK},
    {"info", EVENT_INFO},
    {"queryrequest", EVENT_QUERYREQUEST},
    {"queryresponse", EVENT_QUERYRESPONSE},
    {"ready", EVENT_READY},
    {"exit", EVENT_EXIT},
    {"pipe", EVENT_PIPE},
    {"keypress", EVENT_KEYPRESS},
    {"keyrelease", EVENT_KEYRELEASE},
    {"mousemove", EVENT_MOUSEMOVE},
    {"mousescroll", EVENT_MOUSESCROLL},
    {"mousepress", EVENT_MOUSEPRESS},
    {"mouserelease", EVENT_MOUSERELEASE},
    {"consoledata", EVENT_CONSOLEDATA},
    {"timertick", EVENT_TIMERTICK},
    {"videocmap", EVENT_VIDEOCMAP},
    {"videoconf", EVENT_VIDEOCONF},
    {"videoinfo", EVENT_VIDEOINFO},
    {"blockinfo", EVENT_BLOCKINFO},
    {"blockreadrequest", EVENT_BLOCKREADREQUEST},
    {"blockreadresponse", EVENT_BLOCKREADRESPONSE},
    {"blockwriterequest", EVENT_BLOCKWRITEREQUEST},
    {"blockwriteresponse", EVENT_BLOCKWRITERESPONSE},
    {"clockinfo", EVENT_CLOCKINFO},
    {"ethernetinfo", EVENT_ETHERNETINFO},
    {"loginfo", EVENT_LOGINFO},
    {"walkrequest", EVENT_WALKREQUEST},
    {"walkresponse", EVENT_WALKRESPONSE},
    {"readrequest", EVENT_READREQUEST},
    {"readresponse", EVENT_READRESPONSE},
    {"writerequest", EVENT_WRITEREQUEST},
    {"writeresponse", EVENT_WRITERESPONSE},
    {"statrequest", EVENT_STATREQUEST},
    {"statresponse", EVENT_STATRESPONSE},
    {"maprequest", EVENT_MAPREQUEST},
    {"mapresponse", EVENT_MAPRESPONSE},
    {"createrequest", EVENT_CREATEREQUEST},
    {"createresponse", EVENT_CREATERESPONSE},
    {"removerequest", EVENT_REMOVEREQUEST},
    {"removeresponse", EVENT_REMOVERESPONSE},
    {"wmmap", EVENT_WMMAP},
    {"wmunmap", EVENT_WMUNMAP},
    {"wmgrab", EVENT_WMGRAB},
    {"wmungrab", EVENT_WMUNGRAB},
    {"wmkeypress", EVENT_WMKEYPRESS},
    {"wmkeyrelease", EVENT_WMKEYRELEASE},
    {"wmmousemove", EVENT_WMMOUSEMOVE},
    {"wmmousescroll", EVENT_WMMOUSESCROLL},
    {"wmmousepress", EVENT_WMMOUSEPRESS},
    {"wmmouserelease", EVENT_WMMOUSERELEASE},
    {"wmrenderdata", EVENT_WMRENDERDATA},
    {"wmrenderfile", EVENT_WMRENDERFILE},
    {"wminit", EVENT_WMINIT},
    {"wmevent", EVENT_WMEVENT},
    {"wmack", EVENT_WMACK},
    {"p9p", EVENT_P9P}
};

static unsigned int findevent(char *word)
{

    unsigned int event = 0;
    unsigned int i;

    for (i = 0; i < sizeof (events) / sizeof (events[0]); i++)
    {

        if (cstring_match(word, events[i].name))
            return events[i].event;

    }

    for (i = 0; word[i]; i++)
    {

        if (word[i] < '0' || word[i] > '9')
            return 0;

        event = event * 10 + word[i] - '0';

    }

    return (i && event < 256) ? event : 0;

}

static char *addnumber(struct job *job, unsigned int value)
{

    char *number = job->strings + job->nstrings;

    job->nstrings += cstring_write_fmt(number, JOB_STRINGSSIZE - job->nstrings, 0, "%u\\0", &value);

    return number;

}

static unsigned int addroute(struct job *job, struct job_command *command, unsigned int event, unsigned int to, unsigned int stage)
{

    struct job_route *route;
    unsigned int i;

    if (command->nroutes >= JOB_ROUTES)
        return seterror(job, "Syntax error: Too many redirects", 0);

    for (i = 0; i < command->nroutes; i++)
    {

        if (command->routes[i].event == event)
            return seterror(job, "Syntax error: Event redirected twice", 0);

    }

    route = &command->routes[command->nroutes++];
    route->event = event;
    route->to = to;
    route->stage = stage;

    return 1;

}

static struct job_command *addinput(struct job *job, struct job_command *command, char *path, unsigned int event)
{

    unsigned int i;
    unsigned int j;

    for (i = job->ncommands; i > 0; i--)
        job->commands[i] = job->commands[i - 1];

    job->ncommands++;

    for (i = 1; i < job->ncommands; i++)
    {

        struct job_command *shifted = &job->commands[i];

        if (shifted->side)
            shifted->side++;

        for (j = 0; j < shifted->nroutes; j++)
        {

            if (shifted->routes[j].stage)
                shifted->routes[j].stage++;

        }

    }

    buffer_clear(&job->commands[0], sizeof (struct job_command));

    job->commands[0].paths[0] = path;
    job->commands[0].npaths = 1;

    job->commands[0].program = "play";
    job->commands[0].keys[0] = "event";
    job->commands[0].values[0] = addnumber(job, event);
    job->commands[0].noptions = 1;

    addroute(job, &job->commands[0], event, event, 2);

    return command + 1;

}

static unsigned int addrecord(struct job *job, struct job_command *command, char *path, char *mode, unsigned int event)
{

    struct job_command *record = &job->commands[job->ncommands++];

    record->program = "write";
    record->paths[0] = path;
    record->npaths = 1;
    record->keys[0] = mode;
    record->values[0] = "1";
    record->keys[1] = "event";
    record->values[1] = addnumber(job, event);
    record->noptions = 2;
    record->side = (command - job->commands) + 1;

    return addroute(job, command, event, event, job->ncommands);

}

static unsigned int parse(struct job *job, struct parser *parser)
{

    struct job_command *command = 0;
    unsigned int event = EVENT_DATA;

    for (;;)
    {

        char *word = 0;
        char *value = 0;
        unsigned int token = readtoken(job, parser, &word);

        switch (token)
        {

        case TOKEN_WORD:
            if (command && parser->offset < parser->count && (parser->data[parser->offset] == '<' || parser->data[parser->offset] == '>') && findevent(word))
            {

                event = findevent(word);

                break;

            }

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

            command = addinput(job, command, word, event);
            event = EVENT_DATA;

            break;

        case TOKEN_OUTPUT:
        case TOKEN_APPEND:
            if (!command)
                return seterror(job, "Syntax error: Expected command before > or >>", 0);

            if (job->ncommands >= JOB_COMMANDS)
                return seterror(job, "Syntax error: Too many commands in pipeline", 0);

            if (!readpath(job, parser, &word, (token == TOKEN_APPEND) ? ">>" : ">"))
                return 0;

            if (!addrecord(job, command, word, (token == TOKEN_APPEND) ? "append" : "create", event))
                return 0;

            event = EVENT_DATA;

            break;

        case TOKEN_DUP:
            if (!command)
                return seterror(job, "Syntax error: Expected command before >&", 0);

            if (!readpath(job, parser, &word, ">&"))
                return 0;

            if (!findevent(word))
                return seterror(job, "Syntax error: Unknown event %s", word);

            if (!addroute(job, command, event, findevent(word), 0))
                return 0;

            event = EVENT_DATA;

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

            channel_send_fmt(0, output, EVENT_ERROR, "%s\n", job.error);

            return;

        }

        if (!job.ncommands)
            continue;

        if (!job_spawn(&job, 1, 0, BINPATH))
        {

            channel_send_fmt(0, output, EVENT_ERROR, "%s\n", job.error);
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

static void appenddata(unsigned int source, void *buffer, unsigned int count)
{

    append(buffer, count);

}

static void onpath(struct message *message)
{

    if (fs_read_each(1, message->source, message->data, appenddata))
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
        channel_send_fmt(0, output, EVENT_OPTION, "pwd\\0%s\\0", option_getstring("pwd"));

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

