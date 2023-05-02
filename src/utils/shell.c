#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128

static char inputbuffer[INPUTSIZE];
static struct ring input;
static struct job_worker workers[JOBSIZE];
static struct job job;
static unsigned int keymod = KEYMOD_NONE;
static unsigned int escaped;

static void print(void *buffer, unsigned int count)
{

    file_notify(FILE_G1, EVENT_DATA, count, buffer);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static unsigned int runslang(void *obuffer, unsigned int ocount, void *ibuffer, unsigned int icount)
{

    unsigned int id = file_spawn2(FILE_L0, FILE_G8, option_getstring("slang"));

    if (id)
    {

        char data[MESSAGE_SIZE];
        unsigned int offset = 0;
        unsigned int count;

        channel_listen(id, EVENT_DATA);
        channel_listen(id, EVENT_ERROR);
        channel_listen(id, EVENT_CLOSE);
        channel_sendbuffer(id, EVENT_DATA, icount, ibuffer);
        channel_send(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, EVENT_DATA, data)))
            offset += buffer_write(obuffer, ocount, data, count, offset);

        return offset;

    }

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", option_getstring("slang"));

    return 0;

}

static void interpret(void)
{

    char ibuffer[INPUTSIZE];
    unsigned int icount = ring_read(&input, ibuffer, INPUTSIZE);

    if (icount && (icount = runslang(ibuffer, INPUTSIZE, ibuffer, icount)))
    {

        job_init(&job, workers, JOBSIZE);
        job_parse(&job, ibuffer, icount);

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
                    if (file_walk(FILE_L0, FILE_G8, data))
                        file_duplicate(FILE_G8, FILE_L0);

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

                cstring_writezero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace + lastslash, count - lastspace - lastslash, 0));

                icount = cstring_writefmt3(ibuffer, INPUTSIZE, "/bin/ls %w | /bin/grep ?prefix %s\n", 0, buffer + lastspace, &lastslash, prefix);

            }

            else
            {

                cstring_writezero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace, count - lastspace, 0));

                icount = cstring_writefmt1(ibuffer, INPUTSIZE, "/bin/ls | /bin/grep ?prefix %s\n", 0, prefix);

            }

        }

        else
        {

            cstring_writezero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer, count, 0));

            icount = cstring_writefmt1(ibuffer, INPUTSIZE, "/bin/ls /bin | /bin/grep ?prefix %s\n", 0, prefix);

        }

    }

    else
    {

        cstring_writezero(prefix, INPUTSIZE, 0);

        icount = cstring_writefmt0(ibuffer, INPUTSIZE, "/bin/ls\n", 0);

    }

    return icount;

}

static void complete(void)
{

    char prefix[INPUTSIZE];
    char ibuffer[INPUTSIZE];
    unsigned int icount = createcommand(&input, ibuffer, prefix);

    if (icount && (icount = runslang(ibuffer, INPUTSIZE, ibuffer, icount)))
    {

        job_init(&job, workers, JOBSIZE);
        job_parse(&job, ibuffer, icount);

        if (job_spawn(&job, FILE_L1, FILE_G8))
        {

            struct message message;
            char data[MESSAGE_SIZE];
            struct ring output;

            ring_init(&output, INPUTSIZE, ibuffer);
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

                    char *outputbuffer = ibuffer + cstring_length(prefix);
                    unsigned int outputcount = ring_count(&output) - cstring_lengthzero(prefix);

                    ring_write(&input, outputbuffer, outputcount);
                    print(outputbuffer, outputcount);

                }

                else
                {

                    char tbuffer[INPUTSIZE];

                    print("\n", 1);
                    print(ibuffer, ring_count(&output));
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
                if (!ring_skipreverse(&input, 1))
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
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, keymod);

    keymod = keymap_modkey(keypress->scancode, keymod);

    if (job_count(&job))
    {

        switch (keypress->scancode)
        {

        case 0x2E:
            if ((keymod & KEYMOD_CTRL))
                job_sendfirst(&job, EVENT_TERM, 0, 0);
            else
                job_sendfirst(&job, EVENT_CONSOLEDATA, keycode->length, keycode->value);

            break;

        default:
            job_sendfirst(&job, EVENT_CONSOLEDATA, keycode->length, keycode->value);

            break;

        }

    }

    else
    {

        switch (keypress->scancode)
        {

        case 0x0E:
            if (!ring_skipreverse(&input, 1))
                break;

            print("\b \b", 3);

            break;

        case 0x0F:
            complete();

            break;

        case 0x1C:
            print(keycode->value, keycode->length);
            ring_write(&input, keycode->value, keycode->length);
            interpret();

            break;

        default:
            ring_write(&input, keycode->value, keycode->length);
            print(keycode->value, keycode->length);

            break;

        }

    }

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keymod = keymap_modkey(keyrelease->scancode, keymod);

}
 
static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    print("[ERROR] ", 8);
    print(mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, option_getstring("input")))
        PANIC();

    if (!file_walk2(FILE_G1, option_getstring("output")))
        PANIC();

    file_duplicate(FILE_G8, FILE_PW);
    printprompt();
    file_link(FILE_G0);

    while (channel_process());

    file_unlink(FILE_G0);

}

void init(void)
{

    ring_init(&input, INPUTSIZE, inputbuffer);
    option_add("input", "system:console/if:0/event");
    option_add("output", "system:console/if:0/data");
    option_add("slang", "initrd:/bin/slang");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

}

