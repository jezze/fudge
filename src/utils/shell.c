#include <fudge.h>
#include <abi.h>

#define INPUTSIZE                       128
#define LINESIZE                        (INPUTSIZE * 2)
#define BINPATH                         "initrd:bin"

static char inputdata1[INPUTSIZE];
static struct ring input1;
static char inputdata2[INPUTSIZE];
static struct ring input2;
static char line[LINESIZE];
static unsigned int linecount;
static unsigned int lineoffset;
static struct job job;
static unsigned int interrupts;
static unsigned int newline = 1;
static unsigned int escaped;
static struct keys keys;
static unsigned int console;

static void print(void *buffer, unsigned int count)
{

    channel_send(0, console, EVENT_DATA, count, buffer);

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

            if (job_spawn(&job, 1, 0, BINPATH))
            {

                interrupts = 0;

                job_run(&job, 0, option_getstring("pwd"));

            }

            else
            {

                printfmt1("%s\n", job.error);
                job_abort(&job, 0);

            }

            if (job_count(&job))
                return;

        }

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

static unsigned int runcomplete(char *output, unsigned int size)
{

    unsigned int target = fs_spawn(1, 1, "initrd:bin/complete");
    unsigned int count = 0;

    if (target)
    {

        char input[INPUTSIZE];
        struct message message;

        channel_send_fmt1(1, target, EVENT_OPTION, "pwd=%s\n", option_getstring("pwd"));
        channel_send(1, target, EVENT_MAIN, 0, 0);
        channel_send(1, target, EVENT_DATA, ring_readcopy(&input1, input, INPUTSIZE), input);
        channel_send(1, target, EVENT_TERM, 0, 0);

        while (channel_poll(1, target, EVENT_DATA, &message))
            count += buffer_write(output, size, message.data, message.length, count);

    }

    return count;

}

static void complete(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count = runcomplete(buffer, MESSAGE_SIZE);
    unsigned int length = buffer_findbyte(buffer, count, '\n');

    ring_write(&input1, buffer, length);

    if (length + 1 < count)
    {

        print("\n", 1);
        print(buffer + length + 1, count - length - 1);

    }

}

static void onconsoledata(struct message *message)
{

    struct event_consoledata *consoledata = message->data;

    if (job_count(&job))
    {

        if (consoledata->data == 0x03)
            interrupt();
        else
            job_sendfirst(&job, 0, EVENT_CONSOLEDATA, message->length, message->data);

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

    if (job_count(&job))
    {

        if (keys.mod & KEYS_MOD_CTRL)
        {

            if (id == KEYS_KEY_C)
                interrupt();

        }

        else
        {

            job_sendfirst(&job, 0, EVENT_CONSOLEDATA, keys.code.length, keys.code.value);

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

    if (job_exist(&job, message->source))
        printoutput(message->data, message->length);

}

static void onexit(struct message *message)
{

    struct event_exit *exit = message->data;

    if (job_exit(&job, 0, message->source, exit->status) && !job_count(&job))
        runnext();

}

static void onerror(struct message *message)
{

    print("[ERROR] ", 8);
    printoutput(message->data, message->length);

}

static void onmain(struct message *message)
{

    unsigned int keyboard = channel_lookup(option_getstring("keyboard-service"));

    console = channel_lookup(option_getstring("console-service"));

    if (console)
        channel_send(0, console, EVENT_LINK, 0, 0);

    if (keyboard)
        channel_send(0, keyboard, EVENT_LINK, 0, 0);

    clearline();
    channel_hold(0);

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
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

}

