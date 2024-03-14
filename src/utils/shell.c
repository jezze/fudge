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

    call_notify(FILE_G1, EVENT_DATA, count, buffer);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static unsigned int runslang(void *ibuffer, unsigned int icount)
{

    unsigned int channel = call_spawn_absolute(FILE_L0, FILE_PW, option_getstring("slang"));

    if (channel)
    {

        channel_listen(channel, EVENT_DATA);
        channel_listen(channel, EVENT_ERROR);
        channel_listen(channel, EVENT_CLOSE);
        channel_send_buffer(channel, EVENT_DATA, icount, ibuffer);
        channel_send(channel, EVENT_MAIN);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", option_getstring("slang"));

    }

    return channel;

}

static void interpret(void)
{

    char buffer[INPUTSIZE];
    unsigned int count = ring_read(&input, buffer, INPUTSIZE);
    unsigned int channel = runslang(buffer, count);

    while ((count = channel_read_from(channel, EVENT_DATA, buffer)))
    {

        job_init(&job, workers, JOBSIZE);
        job_parse(&job, buffer, count);

        if (job_spawn(&job, FILE_L1, FILE_G8))
        {

            struct message message;
            char data[MESSAGE_SIZE];

            job_listen(&job, EVENT_CLOSE);
            job_listen(&job, EVENT_DATA);
            job_listen(&job, EVENT_ERROR);
            job_listen(&job, EVENT_PATH);
            job_pipe(&job, EVENT_DATA);
            job_run(&job);

            while (job_pick(&job, &message, data))
            {

                switch (message.event)
                {

                case EVENT_CLOSE:
                    job_close(&job, message.source);

                    break;

                case EVENT_ERROR:
                    channel_dispatch(&message, data);

                    break;

                case EVENT_DATA:
                    print(data, message_datasize(&message));

                    break;

                case EVENT_PATH:
                    if (call_walk_relative(FILE_L0, FILE_G8, data))
                        call_walk_duplicate(FILE_G8, FILE_L0);

                    break;

                }

            }

        }

        else
        {

            job_killall(&job);

        }

    }

    printprompt();

}

static unsigned int createcommand(struct ring *ring, char *ibuffer, char *prefix)
{

    unsigned int icount = 0;

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

                icount = cstring_write_fmt3(ibuffer, INPUTSIZE, "/bin/ls %w | /bin/grep ?prefix %s\n", 0, buffer + lastspace, &lastslash, prefix);

            }

            else
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace, count - lastspace, 0));

                icount = cstring_write_fmt1(ibuffer, INPUTSIZE, "/bin/ls | /bin/grep ?prefix %s\n", 0, prefix);

            }

        }

        else
        {

            cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer, count, 0));

            icount = cstring_write_fmt1(ibuffer, INPUTSIZE, "/bin/ls /bin | /bin/grep ?prefix %s\n", 0, prefix);

        }

    }

    else
    {

        cstring_write_zero(prefix, INPUTSIZE, 0);

        icount = cstring_write_fmt0(ibuffer, INPUTSIZE, "/bin/ls\n", 0);

    }

    return icount;

}

static void complete(void)
{

    char prefix[INPUTSIZE];
    char buffer[INPUTSIZE];
    unsigned int count = createcommand(&input, buffer, prefix);
    unsigned int channel = runslang(buffer, count);

    while ((count = channel_read_from(channel, EVENT_DATA, buffer)))
    {

        job_init(&job, workers, JOBSIZE);
        job_parse(&job, buffer, count);

        if (job_spawn(&job, FILE_L1, FILE_G8))
        {

            struct message message;
            char data[MESSAGE_SIZE];
            struct ring output;

            ring_init(&output, INPUTSIZE, buffer);
            job_listen(&job, EVENT_CLOSE);
            job_listen(&job, EVENT_DATA);
            job_listen(&job, EVENT_ERROR);
            job_pipe(&job, EVENT_DATA);
            job_run(&job);

            while (job_pick(&job, &message, data))
            {

                switch (message.event)
                {

                case EVENT_CLOSE:
                    job_close(&job, message.source);

                    break;

                case EVENT_ERROR:
                    channel_dispatch(&message, data);

                    break;

                case EVENT_DATA:
                    ring_write(&output, data, message_datasize(&message));

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

}

static void clear(void)
{

    char sequence[2] = {0x1B, 'c'};
    char buffer[INPUTSIZE];
    unsigned int count = ring_readcopy(&input, buffer, INPUTSIZE);

    print(sequence, 2);
    printprompt();
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
            job_sendfirst(&job, EVENT_TERM, 0, 0);

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
                    job_sendfirst(&job, EVENT_TERM, 0, 0);

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

    if (!call_walk_absolute(FILE_G0, option_getstring("input")))
        PANIC();

    if (!call_walk_absolute(FILE_G1, option_getstring("output")))
        PANIC();

    call_walk_duplicate(FILE_G8, FILE_PW);
    printprompt();
    call_link(FILE_G0, 8000);

    while (channel_process());

    call_unlink(FILE_G0);

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, KEYS_MAP_US);
    ring_init(&input, INPUTSIZE, inputbuffer);
    option_add("input", "system:console/if.0/event");
    option_add("output", "system:console/if.0/data");
    option_add("slang", "initrd:/bin/slang");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

}

