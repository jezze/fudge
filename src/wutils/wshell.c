#include <fudge.h>
#include <abi.h>

#define INPUTSIZE                       128
#define RESULTSIZE                      2048
#define CONTENTSIZE                     1200
#define LINESIZE                        (INPUTSIZE * 2)
#define BINPATH                         "initrd:bin"

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
static char resultdata[RESULTSIZE];
static struct ring result;
static char line[LINESIZE];
static unsigned int linecount;
static unsigned int lineoffset;
static struct job job;
static unsigned int interrupts;
static struct completion completion;
static unsigned int newline = 1;
static unsigned int wm;

static void print(void *buffer, unsigned int count)
{

    ring_overwrite(&result, buffer, count);

    if (ring_count(&result) >= CONTENTSIZE)
    {

        unsigned int nl;

        ring_skip(&result, ring_count(&result) - CONTENTSIZE + 1);

        nl = ring_find(&result, '\n');

        if (nl)
            ring_skip(&result, nl + 1);

    }

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

static void printring(struct ring *ring)
{

    char buffer[INPUTSIZE];
    unsigned int count = ring_readcopy(ring, buffer, INPUTSIZE);

    if (count)
        print(buffer, count);

}

static unsigned int fitlabel(char *data, unsigned int count, unsigned int size)
{

    unsigned int total = 0;
    unsigned int start = count;

    while (start > 0)
    {

        unsigned int width = (data[start - 1] == '"') ? 4 : 1;

        if (total + width > size)
            break;

        total += width;
        start--;

    }

    return start;

}

static unsigned int writelabel(char *buffer, unsigned int size, unsigned int offset, char *id, char *data, unsigned int count)
{

    unsigned int start;
    unsigned int i;

    offset += cstring_write_fmt1(buffer, size, offset, "= %s label \"", id);
    start = (size > offset + 2) ? fitlabel(data, count, size - offset - 2) : count;

    for (i = start; i < count; i++)
    {

        if (data[i] == '"')
            offset += buffer_write(buffer, size, "\"\\\"\"", 4, offset);
        else
            offset += buffer_write(buffer, size, &data[i], 1, offset);

    }

    offset += buffer_write(buffer, size, "\"\n", 2, offset);

    return offset;

}

static void update(void)
{

    char buffer[MESSAGE_SIZE];
    char content[CONTENTSIZE];
    unsigned int offset = 0;
    unsigned int count;
    unsigned int cursor;

    if (!wm)
        return;

    count = ring_readcopy(&input1, content, CONTENTSIZE);
    cursor = count;
    count += ring_readcopy(&input2, content + count, CONTENTSIZE - count);
    offset += cstring_write_fmt1(buffer, MESSAGE_SIZE, offset, "= output cursor \"%u\"\n", &cursor);
    offset = writelabel(buffer, MESSAGE_SIZE, offset, "input", content, count);
    offset = writelabel(buffer, MESSAGE_SIZE, offset, "prompt", "$ ", (job_count(&job)) ? 0 : 2);
    count = ring_readcopy(&result, content, CONTENTSIZE);
    offset = writelabel(buffer, MESSAGE_SIZE, offset, "result", content, count);

    channel_send(0, wm, EVENT_WMRENDERDATA, offset, buffer);

}

static void showprompt(void)
{

    if (!newline)
        print("\n", 1);

    newline = 1;

    update();

}

static void clearresult(void)
{

    ring_reset(&result);

    newline = 1;

}

static void insertleft(unsigned int count, void *data)
{

    ring_write(&input1, data, count);

}

static void moveleft(unsigned int steps)
{

    char buffer[INPUTSIZE];

    ring_write_reverse(&input2, buffer, ring_read_reverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[INPUTSIZE];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

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

    ring_skip_reverse(&input1, steps);

}

static void deleteright(unsigned int steps)
{

    ring_skip(&input2, steps);

}

static void deletestart(void)
{

    ring_reset(&input1);

}

static void deleteend(void)
{

    ring_reset(&input2);

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

static void interrupt(void)
{

    if (interrupts++)
        job_kill(&job);
    else
        job_sendall(&job, 0, EVENT_INTERRUPT, 0, 0);

    lineoffset = linecount;

}

static void runnext(void)
{

    while (lineoffset < linecount)
    {

        if (!job_parse(&job, line, linecount, &lineoffset))
        {

            printfmt1("%s\n", job.error);

            lineoffset = linecount;

        }

        else if (job.ncommands)
        {

            if (job_spawn(&job, 1, BINPATH))
            {

                interrupts = 0;

                job_run(&job, 0, option_getstring("pwd"));
                update();

                return;

            }

            printfmt1("%s\n", job.error);
            job_abort(&job, 0);

            if (job_count(&job))
                return;

        }

    }

    showprompt();

}

static void submit(void)
{

    linecount = ring_read(&input1, line, LINESIZE - 1);
    linecount += ring_read(&input2, line + linecount, LINESIZE - 1 - linecount);
    line[linecount++] = '\n';
    lineoffset = 0;

    print("$ ", 2);
    print(line, linecount);

    newline = 1;

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

            print("$ ", 2);
            printring(&input1);
            printring(&input2);
            print("\n", 1);
            printoutput(completion.list, completion.listcount);

        }

    }

}

static void ondata(struct message *message)
{

    if (job_exist(&job, message->source))
    {

        printoutput(message->data, message->length);
        update();

    }

}

static void ondone(struct message *message)
{

    job_close(&job, 0, message->source);

}

static void onexit(struct message *message)
{

    if (job_exit(&job, 0, message->source) && !job_count(&job))
        runnext();

}

static void onerror(struct message *message)
{

    print("[ERROR] ", 8);
    printoutput(message->data, message->length);
    update();

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        unsigned int event;

        channel_send(0, wm, EVENT_WMMAP, 0, 0);

        while ((event = channel_process(0)) && event != EVENT_WMCLOSE);

        interrupt();
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wshell.alfi";

    wm = message->source;

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    update();

}

static void onwmkeypress(struct message *message)
{

    struct event_wmkeypress *wmkeypress = message->data;

    wm = message->source;

    if (job_count(&job))
    {

        if (wmkeypress->keymod & KEYS_MOD_CTRL)
        {

            if (wmkeypress->id == KEYS_KEY_C)
                interrupt();

        }

        else
        {

            job_sendfirst(&job, 0, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

        }

        return;

    }

    if (wmkeypress->keymod & KEYS_MOD_CTRL)
    {

        switch (wmkeypress->id)
        {

        case KEYS_KEY_A:
            movestart();

            break;

        case KEYS_KEY_B:
            moveleft(1);

            break;

        case KEYS_KEY_D:
            deleteright(1);

            break;

        case KEYS_KEY_E:
            moveend();

            break;

        case KEYS_KEY_F:
            moveright(1);

            break;

        case KEYS_KEY_H:
            deleteleft(1);

            break;

        case KEYS_KEY_K:
            deleteend();

            break;

        case KEYS_KEY_L:
            clearresult();

            break;

        case KEYS_KEY_U:
            deletestart();

            break;

        }

        update();

        return;

    }

    switch (wmkeypress->id)
    {

    case KEYS_KEY_BACKSPACE:
        deleteleft(1);

        break;

    case KEYS_KEY_DELETE:
        deleteright(1);

        break;

    case KEYS_KEY_TAB:
        complete();

        break;

    case KEYS_KEY_ENTER:
        submit();

        return;

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

    case KEYS_KEY_CURSORUP:
    case KEYS_KEY_CURSORDOWN:
        break;

    default:
        insertleft(wmkeypress->length, &wmkeypress->unicode);

        break;

    }

    update();

}

void init(void)
{

    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    ring_init(&result, RESULTSIZE, resultdata);
    option_add("wm-service", "wm");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_DONE, ondone);
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

    while (channel_process(0));

}

