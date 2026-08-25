#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128

static char inputdata1[INPUTSIZE];
static struct ring input1;
static char inputdata2[INPUTSIZE];
static struct ring input2;
static struct job_worker workers[JOBSIZE];
static struct job job;
static unsigned int escaped;
static struct keys keys;

static void print(void *buffer, unsigned int count)
{

    channel_send_buffer(0, option_getdecimal("console-service"), EVENT_DATA, count, buffer);

}

static void printescape(void *buffer, unsigned int count)
{

    char escape[32];

    escape[0] = 0x1B;

    print(escape, buffer_write(escape, 32, buffer, count, 1) + 1);

}

static void printring(struct ring *ring)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count = ring_readcopy(ring, buffer, MESSAGE_SIZE);

    if (count)
        print(buffer, count);

}

static void printprompt(void)
{

    char buffer[INPUTSIZE];
    unsigned int count = buffer_write(buffer, INPUTSIZE, "$ ", 2, 0);

    count += ring_readcopy(&input1, buffer + count, INPUTSIZE - count);
    count += ring_readcopy(&input2, buffer + count, INPUTSIZE - count);

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

static void insertleft(unsigned int count, void *data)
{

    if (ring_write(&input1, data, count))
        print(data, count);

}

static void insertright(unsigned int count, void *data)
{

    if (ring_write(&input2, data, count))
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

static unsigned int runslang(unsigned int ichannel, void *buffer, unsigned int count)
{

    unsigned int target = fs_spawn(ichannel, option_getstring("slang"));

    if (target)
    {

        channel_send(ichannel, target, EVENT_MAIN);
        channel_send_buffer(ichannel, target, EVENT_DATA, count, buffer);
        channel_send(ichannel, target, EVENT_TERM);

    }

    return target;

}

static void interpretdata(unsigned int ichannel, struct message *message, void *buffer)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message->length);

    if (job_exec(&job, ichannel, "initrd:bin", option_getstring("pwd")))
    {

        char data[MESSAGE_SIZE];

        while (job_pick(&job, ichannel, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, ichannel, message->source, message->event, data, message->length))
                    print(data, message->length);

                break;

            }

        }

    }

}

static void interpret(void)
{

    unsigned char buffer[MESSAGE_SIZE];
    unsigned int count = 0;

    count += ring_read(&input1, buffer + count, MESSAGE_SIZE - count);
    count += ring_read(&input2, buffer + count, MESSAGE_SIZE - count);

    if (count > 1)
    {

        unsigned int channel = runslang(1, buffer, count);

        if (channel)
        {

            struct message message;

            while (channel_poll(1, channel, EVENT_ALL, &message, MESSAGE_SIZE, buffer))
            {

                switch (message.event)
                {

                case EVENT_DATA:
                    interpretdata(0, &message, buffer);

                    break;

                case EVENT_DONE:
                    return;

                }

            }

        }

    }

}

static unsigned int createcommand(char *ibuffer, char *prefix)
{

    unsigned char buffer[INPUTSIZE];
    unsigned int count = 0;

    count += ring_readcopy(&input1, buffer + count, MESSAGE_SIZE - count);
    count += ring_readcopy(&input2, buffer + count, MESSAGE_SIZE - count);

    if (count)
    {

        unsigned int lastspace = buffer_lastbyte(buffer, count, ' ');

        if (lastspace)
        {

            unsigned int lastslash = buffer_lastbyte(buffer + lastspace, count - lastspace, '/');

            if (lastslash)
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace + lastslash, count - lastspace - lastslash, 0));

                return cstring_write_fmt3(ibuffer, INPUTSIZE, 0, "/bin/ls %w | /bin/grep ?prefix %s\n", buffer + lastspace, &lastslash, prefix);

            }

            else
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace, count - lastspace, 0));

                return cstring_write_fmt1(ibuffer, INPUTSIZE, 0, "/bin/ls | /bin/grep ?prefix %s\n", prefix);

            }

        }

        else
        {

            cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer, count, 0));

            return cstring_write_fmt1(ibuffer, INPUTSIZE, 0, "/bin/ls /bin | /bin/grep ?prefix %s\n", prefix);

        }

    }

    else
    {

        cstring_write_zero(prefix, INPUTSIZE, 0);

        return cstring_write_fmt0(ibuffer, INPUTSIZE, 0, "/bin/ls\n");

    }

    return 0;

}

static void completedata(unsigned int ichannel, struct message *message, char *buffer, unsigned int count, char *prefix)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message->length);

    if (job_exec(&job, ichannel, "initrd:bin", option_getstring("pwd")))
    {

        char data[MESSAGE_SIZE];
        struct ring output;

        ring_init(&output, INPUTSIZE, buffer);

        while (job_pick(&job, ichannel, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, ichannel, message->source, message->event, data, message->length))
                    ring_write(&output, data, message->length);

                break;

            }

        }

        if (ring_count(&output))
        {

            if (ring_each(&output, '\n') == ring_count(&output))
            {

                char *outputbuffer = buffer + cstring_length(prefix);
                unsigned int outputcount = ring_count(&output) - cstring_length_zero(prefix);

                ring_write(&input1, outputbuffer, outputcount);

            }

            else
            {

                print("\n", 1);
                printring(&output);

            }

        }

    }

}

static void complete(void)
{

    char prefix[INPUTSIZE];
    char buffer[MESSAGE_SIZE];
    unsigned int count = createcommand(buffer, prefix);

    if (count)
    {

        unsigned int channel = runslang(2, buffer, count);

        if (channel)
        {

            struct message message;

            while (channel_poll(2, channel, EVENT_ALL, &message, MESSAGE_SIZE, buffer))
            {

                switch (message.event)
                {

                case EVENT_DATA:
                    completedata(0, &message, buffer, count, prefix);

                    break;

                case EVENT_DONE:
                    return;

                }

            }

        }

    }

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    if (job_count(&job))
    {

        switch (consoledata->data)
        {

        case 0x03:
            job_sendfirst(&job, 0, EVENT_KILL, 0, 0);

            break;

        default:
            job_sendfirst(&job, 0, EVENT_CONSOLEDATA, msize, mdata);

            break;

        }

    }

    else
    {

        switch (escaped)
        {

        case 1:
            escaped = (consoledata->data == '[') ? 2 : 0;

            break;

        case 2:
            if (consoledata->data >= 0x20 && consoledata->data <= 0x2F)
            {

                /* intermediate bytes */

            }

            if (consoledata->data >= 0x30 && consoledata->data <= 0x3F)
            {

                /* parameter bytes */

            }

            if (consoledata->data >= 0x40 && consoledata->data <= 0x7E)
            {

                escaped = 0;

            }

            break;

        default:
            switch (consoledata->data)
            {

            case 0x00:
                break;

            case 0x01:
                movestart();

                break;

            case 0x02:
                moveleft(1);

                break;

            case 0x03:
                break;

            case 0x04:
                break;

            case 0x05:
                moveend();

                break;

            case 0x06:
                moveright(1);

                break;

            case 0x07:
                break;

            case 0x08:
                deleteleft(1);

                break;

            case 0x09:
                complete();
                clearline();

                break;

            case 0x0A:
                insertright(1, "\n");
                interpret();
                clearline();

                break;

            case 0x0B:
                deleteend();

                break;

            case 0x0C:
                clearscreen();

                break;

            case 0x0D:
                insertright(1, "\n");
                interpret();
                clearline();

                break;

            case 0x0E:
                break;

            case 0x0F:
                break;

            case 0x10:
                break;

            case 0x11:
                break;

            case 0x12:
                break;

            case 0x13:
                break;

            case 0x14:
                break;

            case 0x15:
                deletestart();

                break;

            case 0x16:
                break;

            case 0x17:
                break;

            case 0x18:
                break;

            case 0x19:
                break;

            case 0x1A:
                break;

            case 0x1B:
                escaped = 1;

                break;

            case 0x1C:
                break;

            case 0x1D:
                break;

            case 0x1E:
                break;

            case 0x1F:
                break;

            case 0x7F:
                deleteleft(1);

                break;

            default:
                insertleft(1, &consoledata->data);

                break;

            }

            break;

        }

    }

}

static void onkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keypress *keypress = mdata;
    unsigned int id = keys_getcode(&keys, keypress->scancode);

    if (id)
    {

        if (job_count(&job))
        {

            if (keys.mod & KEYS_MOD_CTRL)
            {

                switch (id)
                {

                case KEYS_KEY_C:
                    job_sendfirst(&job, 0, EVENT_KILL, 0, 0);

                    break;

                }

            }

            else
            {

                job_sendfirst(&job, 0, EVENT_CONSOLEDATA, keys.code.length, keys.code.value);

            }

        }

        else
        {

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
                insertright(keys.code.length, keys.code.value);
                interpret();
                clearline();

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

    }

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keys_getcode(&keys, keyrelease->scancode);

}

static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    print("[ERROR] ", 8);
    print(mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    option_setdecimal("console-service", channel_lookup(option_getstring("console-service")));
    option_setdecimal("keyboard-service", channel_lookup(option_getstring("keyboard-service")));
    channel_send(0, option_getdecimal("console-service"), EVENT_LINK);
    channel_send(0, option_getdecimal("keyboard-service"), EVENT_LINK);
    clearline();

    while (channel_process(0));

    channel_send(0, option_getdecimal("console-service"), EVENT_UNLINK);
    channel_send(0, option_getdecimal("keyboard-service"), EVENT_UNLINK);

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, KEYS_MAP_US);
    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    option_add("slang", "initrd:bin/slang");
    option_add("console-service", "console");
    option_add("keyboard-service", "keyboard");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

