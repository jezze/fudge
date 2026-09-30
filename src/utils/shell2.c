#include <fudge.h>
#include <abi.h>

#define INPUTSIZE                       128
#define LINESIZE                        (INPUTSIZE * 2)
#define STRINGSSIZE                     (LINESIZE * 2)
#define COMMANDS                        8
#define PATHS                           16
#define OPTIONS                         16
#define BINPATH                         "initrd:bin"
#define TOKEN_END                       1
#define TOKEN_WORD                      2
#define TOKEN_OPTION                    3
#define TOKEN_PIPE                      4
#define TOKEN_ERROR                     5

struct command
{

    char *program;
    char *paths[PATHS];
    unsigned int npaths;
    char *keys[OPTIONS];
    char *values[OPTIONS];
    unsigned int noptions;

};

struct pipeline
{

    struct command commands[COMMANDS];
    unsigned int ncommands;
    char strings[STRINGSSIZE];
    unsigned int nstrings;

};

struct completion
{

    char common[RECORD_NAMESIZE];
    unsigned int commoncount;
    unsigned int isdirectory;
    unsigned int nmatches;
    char list[MESSAGE_SIZE];
    unsigned int listcount;

};

static char inputdata1[INPUTSIZE];
static struct ring input1;
static char inputdata2[INPUTSIZE];
static struct ring input2;
static char line[LINESIZE];
static unsigned int linecount;
static unsigned int lineoffset;
static char *parseerror;
static struct pipeline pipeline;
static struct completion completion;
static unsigned int processes[COMMANDS];
static unsigned int nprocesses;
static unsigned int newline = 1;
static unsigned int escaped;
static struct keys keys;

static void print(void *buffer, unsigned int count)
{

    unsigned int target = channel_lookup(option_getstring("console-service"));

    channel_send(0, target, EVENT_DATA, count, buffer);

}

static void printfmt1(char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    print(buffer, cstring_write_fmt1(buffer, MESSAGE_SIZE, 0, fmt, arg1));

}

static void printoutput(void *buffer, unsigned int count)
{

    if (count)
    {

        char *data = buffer;

        print(buffer, count);

        newline = (data[count - 1] == '\n');

    }

}

static void printescape(void *buffer, unsigned int count)
{

    char escape[32];

    escape[0] = 0x1B;

    print(escape, buffer_write(escape, 32, buffer, count, 1) + 1);

}

static void printprompt(void)
{

    char buffer[INPUTSIZE * 2];
    unsigned int count = buffer_write(buffer, INPUTSIZE * 2, "$ ", 2, 0);

    count += ring_readcopy(&input1, buffer + count, INPUTSIZE * 2 - count);
    count += ring_readcopy(&input2, buffer + count, INPUTSIZE * 2 - count);

    print(buffer, count);

}

static void cursorleft(unsigned int steps)
{

    if (steps)
    {

        unsigned char num[32];

        printescape(num, cstring_write_fmt1(num, 32, 0, "[%uD", &steps));

    }

}

static void cursorright(unsigned int steps)
{

    if (steps)
    {

        unsigned char num[32];

        printescape(num, cstring_write_fmt1(num, 32, 0, "[%uC", &steps));

    }

}

static void clearline(void)
{

    printescape("[2K", 3);
    printescape("[0G", 3);
    printprompt();
    cursorleft(ring_count(&input2));

}

static void clearscreen(void)
{

    printescape("[2J", 3);
    printescape("[H", 2);
    printprompt();
    cursorleft(ring_count(&input2));

}

static void showprompt(void)
{

    if (!newline)
        print("\n", 1);

    newline = 1;

    clearline();

}

static void insertleft(unsigned int count, void *data)
{

    if (ring_write(&input1, data, count))
        print(data, count);

}

static void moveleft(unsigned int steps)
{

    char buffer[INPUTSIZE];

    cursorleft(ring_write_reverse(&input2, buffer, ring_read_reverse(&input1, buffer, steps)));

}

static void moveright(unsigned int steps)
{

    char buffer[INPUTSIZE];

    cursorright(ring_write(&input1, buffer, ring_read(&input2, buffer, steps)));

}

static void movestart(void)
{

    moveleft(ring_count(&input1));

}

static void moveend(void)
{

    moveright(ring_count(&input2));

}

static void deleteleft(unsigned int steps)
{

    if (ring_skip_reverse(&input1, steps))
        clearline();

}

static void deleteright(unsigned int steps)
{

    if (ring_skip(&input2, steps))
        clearline();

}

static void deletestart(void)
{

    deleteleft(ring_count(&input1));

}

static void deleteend(void)
{

    deleteright(ring_count(&input2));

}

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

static void addchar(struct pipeline *p, char c)
{

    if (p->nstrings < STRINGSSIZE)
        p->strings[p->nstrings++] = c;

}

static unsigned int readtoken(struct pipeline *p, char **word)
{

    unsigned int type = TOKEN_WORD;

    while (lineoffset < linecount && isblankchar(line[lineoffset]))
        lineoffset++;

    if (lineoffset >= linecount)
        return TOKEN_END;

    switch (line[lineoffset])
    {

    case '|':
        lineoffset++;

        return TOKEN_PIPE;

    case ';':
    case '\n':
        lineoffset++;

        return TOKEN_END;

    case '-':
        lineoffset++;

        type = TOKEN_OPTION;

        break;

    }

    *word = p->strings + p->nstrings;

    while (lineoffset < linecount && !isspecialchar(line[lineoffset]))
    {

        char c = line[lineoffset++];

        if (c == '"' || c == '\'')
        {

            while (lineoffset < linecount && line[lineoffset] != c)
                addchar(p, line[lineoffset++]);

            if (lineoffset >= linecount)
            {

                parseerror = "Unterminated quote";

                return TOKEN_ERROR;

            }

            lineoffset++;

        }

        else
        {

            addchar(p, c);

        }

    }

    addchar(p, '\0');

    if (type == TOKEN_OPTION && !cstring_length(*word))
    {

        parseerror = "Expected option name after -";

        return TOKEN_ERROR;

    }

    return type;

}

static unsigned int syntaxerror(char *message)
{

    printfmt1("Syntax error: %s\n", message);

    return 0;

}

static unsigned int parsepipeline(struct pipeline *p)
{

    struct command *command = 0;

    buffer_clear(p, sizeof (struct pipeline));

    for (;;)
    {

        char *word = 0;
        char *value = 0;

        switch (readtoken(p, &word))
        {

        case TOKEN_WORD:
            if (command)
            {

                if (command->npaths >= PATHS)
                    return syntaxerror("Too many arguments");

                command->paths[command->npaths++] = word;

            }

            else
            {

                if (p->ncommands >= COMMANDS)
                    return syntaxerror("Too many commands in pipeline");

                command = &p->commands[p->ncommands++];
                command->program = word;

            }

            break;

        case TOKEN_OPTION:
            if (!command)
                return syntaxerror("Expected command before option");

            if (command->noptions >= OPTIONS)
                return syntaxerror("Too many options");

            switch (readtoken(p, &value))
            {

            case TOKEN_WORD:
                command->keys[command->noptions] = word;
                command->values[command->noptions] = value;
                command->noptions++;

                break;

            case TOKEN_ERROR:
                return syntaxerror(parseerror);

            default:
                printfmt1("Syntax error: Expected value after -%s\n", word);

                return 0;

            }

            break;

        case TOKEN_PIPE:
            if (!command)
                return syntaxerror("Expected command before |");

            command = 0;

            break;

        case TOKEN_END:
            if (p->ncommands && !command)
                return syntaxerror("Expected command after |");

            return 1;

        case TOKEN_ERROR:
            return syntaxerror(parseerror);

        }

    }

}

static unsigned int findprocess(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < nprocesses; i++)
    {

        if (processes[i] == source)
            return i;

    }

    return nprocesses;

}

static unsigned int countprocesses(void)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < nprocesses; i++)
    {

        if (processes[i])
            count++;

    }

    return count;

}

static unsigned int firstprocess(void)
{

    unsigned int i;

    for (i = 0; i < nprocesses; i++)
    {

        if (processes[i])
            return processes[i];

    }

    return 0;

}

static void activate(unsigned int start)
{

    unsigned int i;

    for (i = start; i < nprocesses; i++)
    {

        if (processes[i])
        {

            channel_send(0, processes[i], EVENT_TERM, 0, 0);

            break;

        }

    }

}

static void interrupt(void)
{

    unsigned int i;

    for (i = 0; i < nprocesses; i++)
    {

        if (processes[i])
            channel_send(0, processes[i], EVENT_INTERRUPT, 0, 0);

    }

    lineoffset = linecount;

}

static void abortspawned(unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (processes[i])
            channel_send(0, processes[i], EVENT_TERM, 0, 0);

        processes[i] = 0;

    }

    nprocesses = 0;

}

static unsigned int spawn(char *program)
{

    unsigned int target = fs_spawn_relative(1, program, BINPATH);

    if (!target)
        target = fs_spawn(1, program);

    return target;

}

static unsigned int startpipeline(struct pipeline *p)
{

    char *pwd = option_getstring("pwd");
    unsigned int i;

    for (i = 0; i < p->ncommands; i++)
    {

        processes[i] = spawn(p->commands[i].program);

        if (!processes[i])
        {

            printfmt1("Command not found: %s\n", p->commands[i].program);
            abortspawned(i);

            return 0;

        }

    }

    nprocesses = p->ncommands;

    for (i = 0; i < nprocesses; i++)
    {

        struct command *command = &p->commands[i];
        unsigned int j;

        channel_send_fmt1(0, processes[i], EVENT_OPTION, "pwd=%s\n", pwd);

        for (j = 0; j < command->noptions; j++)
            channel_send_fmt2(0, processes[i], EVENT_OPTION, "%s=%s\n", command->keys[j], command->values[j]);

    }

    for (i = 0; i < nprocesses; i++)
        channel_send(0, processes[i], EVENT_MAIN, 0, 0);

    for (i = 0; i < nprocesses; i++)
    {

        struct command *command = &p->commands[i];
        unsigned int j;

        for (j = 0; j < command->npaths; j++)
        {

            char *path = command->paths[j];

            if (fs_auth(path))
                channel_send_fmt1(0, processes[i], EVENT_PATH, "%s\\0", path);
            else
                channel_send_fmt2(0, processes[i], EVENT_PATH, "%s%s\\0", pwd, path);

        }

    }

    activate(0);

    return 1;

}

static void runnext(void)
{

    while (lineoffset < linecount)
    {

        if (!parsepipeline(&pipeline))
            lineoffset = linecount;
        else if (pipeline.ncommands && startpipeline(&pipeline))
            return;

    }

    showprompt();

}

static void submit(void)
{

    print("\n", 1);

    linecount = ring_read(&input1, line, LINESIZE - 1);
    linecount += ring_read(&input2, line + linecount, LINESIZE - 1 - linecount);
    line[linecount++] = '\n';
    lineoffset = 0;

    runnext();

}

static unsigned int isdotentry(struct record *record)
{

    if (record->length == 1 && record->name[0] == '.')
        return 1;

    if (record->length == 2 && record->name[0] == '.' && record->name[1] == '.')
        return 1;

    return 0;

}

static void addmatch(struct completion *c, struct record *record)
{

    unsigned int length = (record->length < RECORD_NAMESIZE) ? record->length : RECORD_NAMESIZE;

    if (c->nmatches)
    {

        unsigned int i;

        for (i = 0; i < c->commoncount && i < length && c->common[i] == record->name[i]; i++);

        c->commoncount = i;

    }

    else
    {

        buffer_copy(c->common, record->name, length);

        c->commoncount = length;
        c->isdirectory = (record->type == RECORD_TYPE_DIRECTORY);

    }

    if (record->type == RECORD_TYPE_DIRECTORY)
        c->listcount += cstring_write_fmt2(c->list, MESSAGE_SIZE, c->listcount, "%w/\n", record->name, &length);
    else
        c->listcount += cstring_write_fmt2(c->list, MESSAGE_SIZE, c->listcount, "%w\n", record->name, &length);

    c->nmatches++;

}

static void scandirectory(struct completion *c, char *directory, char *prefix, unsigned int prefixcount)
{

    unsigned int target = fs_auth(directory);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, directory);

        if (id)
        {

            unsigned char data[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset = 0;

            while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
            {

                unsigned int i;

                for (i = 0; i < count; i += sizeof (struct record))
                {

                    struct record *record = (struct record *)(data + i);

                    if (record->length >= prefixcount && buffer_match(record->name, prefix, prefixcount) && !isdotentry(record))
                        addmatch(c, record);

                    offset = record->offset;

                }

            }

        }

    }

}

static void complete(void)
{

    char buffer[INPUTSIZE];
    char directory[LINESIZE];
    unsigned int count = ring_readcopy(&input1, buffer, INPUTSIZE);
    unsigned int start;
    unsigned int split;
    unsigned int prefixcount;
    unsigned int dircount;
    unsigned int command;
    unsigned int i;

    for (start = count; start > 0 && !isspecialchar(buffer[start - 1]); start--);
    for (i = start; i > 0 && isblankchar(buffer[i - 1]); i--);
    for (split = count; split > start && buffer[split - 1] != '/' && buffer[split - 1] != ':'; split--);

    command = (i == 0 || buffer[i - 1] == '|' || buffer[i - 1] == ';');
    prefixcount = count - split;
    dircount = split - start;

    if (dircount)
    {

        cstring_write_fmt2(directory, LINESIZE, 0, "%w\\0", buffer + start, &dircount);

        if (!fs_auth(directory))
            cstring_write_fmt3(directory, LINESIZE, 0, "%s%w\\0", option_getstring("pwd"), buffer + start, &dircount);

    }

    else
    {

        cstring_write_fmt1(directory, LINESIZE, 0, "%s\\0", (command) ? BINPATH : option_getstring("pwd"));

    }

    buffer_clear(&completion, sizeof (struct completion));
    scandirectory(&completion, directory, buffer + split, prefixcount);

    if (completion.nmatches == 1)
    {

        ring_write(&input1, completion.common + prefixcount, completion.commoncount - prefixcount);
        ring_write(&input1, (completion.isdirectory) ? "/" : " ", 1);

    }

    else if (completion.nmatches > 1)
    {

        if (completion.commoncount > prefixcount)
        {

            ring_write(&input1, completion.common + prefixcount, completion.commoncount - prefixcount);

        }

        else
        {

            print("\n", 1);
            print(completion.list, completion.listcount);

        }

    }

}

static void onconsoledata(struct message *message)
{

    struct event_consoledata *consoledata = message->data;

    if (countprocesses())
    {

        if (consoledata->data == 0x03)
            interrupt();
        else
            channel_send(0, firstprocess(), EVENT_CONSOLEDATA, message->length, message->data);

        return;

    }

    switch (escaped)
    {

    case 1:
        escaped = (consoledata->data == '[') ? 2 : 0;

        break;

    case 2:
        if (consoledata->data >= 0x40 && consoledata->data <= 0x7E)
        {

            switch (consoledata->data)
            {

            case 'C':
                moveright(1);

                break;

            case 'D':
                moveleft(1);

                break;

            case 'F':
                moveend();

                break;

            case 'H':
                movestart();

                break;

            }

            escaped = 0;

        }

        break;

    default:
        switch (consoledata->data)
        {

        case 0x01:
            movestart();

            break;

        case 0x02:
            moveleft(1);

            break;

        case 0x05:
            moveend();

            break;

        case 0x06:
            moveright(1);

            break;

        case 0x08:
        case 0x7F:
            deleteleft(1);

            break;

        case 0x09:
            complete();
            clearline();

            break;

        case 0x0A:
        case 0x0D:
            submit();

            break;

        case 0x0B:
            deleteend();

            break;

        case 0x0C:
            clearscreen();

            break;

        case 0x15:
            deletestart();

            break;

        case 0x1B:
            escaped = 1;

            break;

        default:
            if (consoledata->data >= 0x20)
                insertleft(1, &consoledata->data);

            break;

        }

        break;

    }

}

static void onkeypress(struct message *message)
{

    struct event_keypress *keypress = message->data;
    unsigned int id = keys_getcode(&keys, keypress->scancode);

    if (!id)
        return;

    if (countprocesses())
    {

        if (keys.mod & KEYS_MOD_CTRL)
        {

            if (id == KEYS_KEY_C)
                interrupt();

        }

        else
        {

            channel_send(0, firstprocess(), EVENT_CONSOLEDATA, keys.code.length, keys.code.value);

        }

        return;

    }

    switch (id)
    {

    case KEYS_KEY_BACKSPACE:
        deleteleft(1);

        break;

    case KEYS_KEY_TAB:
        complete();
        clearline();

        break;

    case KEYS_KEY_ENTER:
        submit();

        break;

    case KEYS_KEY_HOME:
        movestart();

        break;

    case KEYS_KEY_CURSORLEFT:
        moveleft(1);

        break;

    case KEYS_KEY_CURSORRIGHT:
        moveright(1);

        break;

    case KEYS_KEY_END:
        moveend();

        break;

    default:
        insertleft(keys.code.length, keys.code.value);

        break;

    }

}

static void onkeyrelease(struct message *message)
{

    struct event_keyrelease *keyrelease = message->data;

    keys_getcode(&keys, keyrelease->scancode);

}

static void ondata(struct message *message)
{

    unsigned int index = findprocess(message->source);

    if (index < nprocesses)
    {

        if (index + 1 < nprocesses)
        {

            if (processes[index + 1])
                channel_send(0, processes[index + 1], EVENT_DATA, message->length, message->data);

        }

        else
        {

            printoutput(message->data, message->length);

        }

    }

}

static void ondone(struct message *message)
{

    unsigned int index = findprocess(message->source);

    if (index < nprocesses)
    {

        processes[index] = 0;

        activate(index + 1);

        if (!countprocesses())
        {

            nprocesses = 0;

            runnext();

        }

    }

}

static void onerror(struct message *message)
{

    print("[ERROR] ", 8);
    printoutput(message->data, message->length);

}

static void onmain(struct message *message)
{

    unsigned int console = channel_lookup(option_getstring("console-service"));
    unsigned int keyboard = channel_lookup(option_getstring("keyboard-service"));

    if (console)
        channel_send(0, console, EVENT_LINK, 0, 0);

    if (keyboard)
        channel_send(0, keyboard, EVENT_LINK, 0, 0);

    clearline();

    while (channel_process(0));

    if (console)
        channel_send(0, console, EVENT_UNLINK, 0, 0);

    if (keyboard)
        channel_send(0, keyboard, EVENT_UNLINK, 0, 0);

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, KEYS_MAP_US);
    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    option_add("console-service", "console");
    option_add("keyboard-service", "keyboard");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_DONE, ondone);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

