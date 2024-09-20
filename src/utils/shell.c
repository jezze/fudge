#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128

static char inputbuffer[INPUTSIZE];
static struct ring input;
static struct job_worker workers[JOBSIZE];
static struct job job;
static unsigned int escaped;
static struct keys keys;

static void print(void *buffer, unsigned int count)
{

    channel_send_buffer(option_getdecimal("tty-service"), EVENT_DATA, count, buffer);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static unsigned int runslang(void *ibuffer, unsigned int icount)
{

    unsigned int channel = fs_spawn(option_getstring("slang"));

    if (channel)
    {

        channel_send(channel, EVENT_MAIN);
        channel_send_buffer(channel, EVENT_DATA, icount, ibuffer);
        channel_send(channel, EVENT_END);

    }

    return channel;

}

static void interpretdata(struct message *message, void *buffer)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message_datasize(message));

    if (job_spawn(&job, "initrd:bin"))
    {

        char data[MESSAGE_SIZE];

        job_run(&job, option_getstring("env"), option_getstring("pwd"));

        while (job_pick(&job, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, message->source, message->event, data, message_datasize(message)))
                    print(data, message_datasize(message));

                break;

            }

        }

        printprompt();

    }

    else
    {

        job_killall(&job);
        printprompt();

    }

}

static void interpret(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count = ring_read(&input, buffer, MESSAGE_SIZE);

    if (count > 1)
    {

        struct message message;
        unsigned int channel = runslang(buffer, count);

        while (channel_pollany(channel, &message, MESSAGE_SIZE, buffer))
        {

            switch (message.event)
            {

            case EVENT_DATA:
                interpretdata(&message, buffer);

                break;

            case EVENT_TERMRESPONSE:
                return;

            }

        }

    }

    else
    {

        printprompt();

    }

}

static unsigned int createcommand(struct ring *ring, char *ibuffer, char *prefix)
{

    if (ring_count(ring))
    {

        char buffer[INPUTSIZE];
        unsigned int count = ring_readcopy(ring, buffer, INPUTSIZE);
        unsigned int lastspace = buffer_lastbyte(buffer, count, ' ');

        if (lastspace)
        {

            unsigned int lastslash = buffer_lastbyte(buffer + lastspace, count - lastspace, '/');

            if (lastslash)
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace + lastslash, count - lastspace - lastslash, 0));

                return cstring_write_fmt3(ibuffer, INPUTSIZE, "/bin/ls %w | /bin/grep ?prefix %s\n", 0, buffer + lastspace, &lastslash, prefix);

            }

            else
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace, count - lastspace, 0));

                return cstring_write_fmt1(ibuffer, INPUTSIZE, "/bin/ls | /bin/grep ?prefix %s\n", 0, prefix);

            }

        }

        else
        {

            cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer, count, 0));

            return cstring_write_fmt1(ibuffer, INPUTSIZE, "/bin/ls /bin | /bin/grep ?prefix %s\n", 0, prefix);

        }

    }

    else
    {

        cstring_write_zero(prefix, INPUTSIZE, 0);

        return cstring_write_fmt0(ibuffer, INPUTSIZE, "/bin/ls\n", 0);

    }

    return 0;

}

static void completedata(struct message *message, char *buffer, unsigned int count, char *prefix)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message_datasize(message));

    if (job_spawn(&job, "initrd:bin"))
    {

        char data[MESSAGE_SIZE];
        struct ring output;

        ring_init(&output, INPUTSIZE, buffer);
        job_run(&job, option_getstring("env"), option_getstring("pwd"));

        while (job_pick(&job, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, message->source, message->event, data, message_datasize(message)))
                    ring_write(&output, data, message_datasize(message));

                break;

            }

        }

        if (ring_count(&output))
        {

            if (ring_each(&output, '\n') == ring_count(&output))
            {

                char *outputbuffer = buffer + cstring_length(prefix);
                unsigned int outputcount = ring_count(&output) - cstring_length_zero(prefix);

                ring_write(&input, outputbuffer, outputcount);
                print(outputbuffer, outputcount);

            }

            else
            {

                char tbuffer[INPUTSIZE];

                print("\n", 1);
                print(buffer, ring_count(&output));
                printprompt();
                print(tbuffer, ring_readcopy(&input, tbuffer, INPUTSIZE));

            }

        }

    }

    else
    {

        job_killall(&job);

    }

}

static void complete(void)
{

    struct message message;
    char prefix[INPUTSIZE];
    char buffer[MESSAGE_SIZE];
    unsigned int count = createcommand(&input, buffer, prefix);
    unsigned int channel = runslang(buffer, count);

    while (channel_pollany(channel, &message, MESSAGE_SIZE, buffer))
    {

        switch (message.event)
        {

        case EVENT_DATA:
            completedata(&message, buffer, count, prefix);

            break;

        case EVENT_TERMRESPONSE:
            return;

        }

    }

}

static void clear(void)
{

    char sequence[2] = {0x1B, 'c'};
    char buffer[INPUTSIZE];
    unsigned int count = ring_readcopy(&input, buffer, INPUTSIZE);

    print(sequence, 2);
    printprompt();

    if (count)
        print(buffer, count);

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    if (job_count(&job))
    {

        switch (consoledata->data)
        {

        case 0x03:
            job_sendfirst(&job, EVENT_TERMREQUEST, 0, 0);

            break;

        default:
            job_sendfirst(&job, EVENT_CONSOLEDATA, msize, mdata);

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

            case '\0':
                break;

            case 0x1B:
                escaped = 1;

                break;

            case '\f':
                clear();

                break;

            case '\t':
                complete();

                break;

            case '\b':
            case 0x7F:
                if (!ring_skip_reverse(&input, 1))
                    break;

                print("\b \b", 3);

                break;

            case '\r':
                consoledata->data = '\n';

            case '\n':
                print(&consoledata->data, 1);
                ring_write(&input, &consoledata->data, 1);
                interpret();

                break;

            default:
                ring_write(&input, &consoledata->data, 1);
                print(&consoledata->data, 1);

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
                    job_sendfirst(&job, EVENT_TERMREQUEST, 0, 0);

                    break;

                }

            }

            else
            {

                job_sendfirst(&job, EVENT_CONSOLEDATA, keys.code.length, keys.code.value);

            }

        }

        else
        {

            switch (id)
            {

            case KEYS_KEY_BACKSPACE:
                if (!ring_skip_reverse(&input, 1))
                    break;

                print("\b \b", 3);

                break;

            case KEYS_KEY_TAB:
                complete();

                break;

            case KEYS_KEY_ENTER:
                print(keys.code.value, keys.code.length);
                ring_write(&input, keys.code.value, keys.code.length);
                interpret();

                break;

            default:
                ring_write(&input, keys.code.value, keys.code.length);
                print(keys.code.value, keys.code.length);

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

    printprompt();
    channel_send(option_getdecimal("tty-service"), EVENT_LINK);
    channel_send(option_getdecimal("keyboard-service"), EVENT_LINK);

    while (channel_process());

    channel_send(option_getdecimal("tty-service"), EVENT_UNLINK);
    channel_send(option_getdecimal("keyboard-service"), EVENT_UNLINK);

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, KEYS_MAP_US);
    ring_init(&input, INPUTSIZE, inputbuffer);
    option_add("slang", "initrd:bin/slang");
    option_add("tty-service", "");
    option_add("keyboard-service", "");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

}

