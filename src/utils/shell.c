#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128

static char inputbuffer[INPUTSIZE];
static struct ring input;
static struct job_worker workers[JOBSIZE];
static struct job job;
static unsigned int keymod = KEYMOD_NONE;

static void print(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, buffer, count);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void interpret(void)
{

    char buffer[INPUTSIZE];
    unsigned int count = ring_read(&input, buffer, INPUTSIZE);

    if (count)
    {

        unsigned int id = file_spawn("/bin/slang");

        if (id)
        {

            struct message message;

            job_init(&job, workers, JOBSIZE);
            channel_redirectback(id, EVENT_DATA);
            channel_redirectback(id, EVENT_ERROR);
            channel_redirectback(id, EVENT_CLOSE);
            channel_sendbufferto(id, EVENT_DATA, count, buffer);
            channel_sendto(id, EVENT_MAIN);

            while ((count = channel_readfrom(id, message.data.buffer, MESSAGE_SIZE)))
                job_parse(&job, message.data.buffer, count);

            job_spawn(&job);
            job_pipe(&job);
            job_run(&job);

            while (job_pick(&job, &message))
            {

                switch (message.header.event)
                {

                case EVENT_CLOSE:
                    job_close(&job, message.header.source);

                    break;

                case EVENT_DATA:
                    print(message.data.buffer, message_datasize(&message.header));

                    break;

                case EVENT_PATH:
                    if (file_walk(FILE_L0, FILE_CW, message.data.buffer))
                        file_duplicate(FILE_CW, FILE_L0);

                    break;

                }

            }

        }

    }

    printprompt();

}

static char prefix[INPUTSIZE];
static unsigned int prefixcount;

static void completespawn(unsigned int count, void *buffer)
{

    job.workers[0].program = "/bin/ls";
    job.count = 1;

    if (count)
    {

        unsigned int lastwordoffset = buffer_lastbyte(buffer, count, ' ');
        unsigned int lastwordcount = count - lastwordoffset;
        unsigned int searchoffset = lastwordoffset + buffer_lastbyte((char *)buffer + lastwordoffset, lastwordcount, '/');
        unsigned int searchcount = count - searchoffset;

        if (lastwordoffset)
        {

            if (searchoffset > lastwordoffset)
            {

                char path[INPUTSIZE];
                unsigned int pathcount;

                pathcount = buffer_write(path, INPUTSIZE, (char *)buffer + lastwordoffset, searchoffset - lastwordoffset - 1, 0);
                pathcount += cstring_writez(path, INPUTSIZE, "", pathcount);

                job.workers[0].paths[0] = path;
                job.workers[0].npaths = 1;

            }

        }

        else
        {

            job.workers[0].paths[0] = "/bin";
            job.workers[0].npaths = 1;

        }

        if (searchcount)
        {

            prefixcount = buffer_write(prefix, INPUTSIZE, (char *)buffer + searchoffset, searchcount, 0);
            prefixcount += cstring_writez(prefix, INPUTSIZE, "", prefixcount);

            if (prefixcount)
            {

                job.workers[1].program = "/bin/grep";
                job.workers[1].options[0].key = "prefix";
                job.workers[1].options[0].value = prefix;
                job.workers[1].noptions = 1;
                job.count = 2;

            }

        }

    }

}

static void completeprocess(void)
{

    char buffer[BUFFER_SIZE];
    struct ring result;
    struct message message;

    ring_init(&result, BUFFER_SIZE, buffer);

    while (job_pick(&job, &message))
    {

        switch (message.header.event)
        {

        case EVENT_CLOSE:
            job_close(&job, message.header.source);

            break;

        case EVENT_DATA:
            ring_write(&result, message.data.buffer, message_datasize(&message.header));

            break;

        }

    }

    if (ring_each(&result, '\n') == ring_count(&result))
    {

        ring_write(&input, buffer + cstring_length(prefix), ring_count(&result) - prefixcount);
        print(buffer + cstring_length(prefix), ring_count(&result) - prefixcount);


    }

    else
    {

        print("\n", 1);
        print(buffer, ring_count(&result));
        printprompt();
        print(buffer, ring_readcopy(&input, buffer, INPUTSIZE));

    }

}

static void complete(void)
{

    char buffer[INPUTSIZE];
    unsigned int count = ring_readcopy(&input, buffer, INPUTSIZE);

    job_init(&job, workers, JOBSIZE);
    completespawn(count, buffer);
    job_spawn(&job);
    job_pipe(&job);
    job_run(&job);
    completeprocess();

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

        switch (consoledata->data)
        {

        case '\0':
            break;

        case '\f':
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

    print("Error: ", 7);
    print(mdata, msize);
    print("\n", 1);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, option_getstring("input")))
        channel_warning("Could not get input device");

    if (!file_walk2(FILE_G1, option_getstring("output")))
        channel_warning("Could not get output device");

    printprompt();
    file_link(FILE_G0);

    while (channel_process());

    file_unlink(FILE_G0);

}

void init(void)
{

    ring_init(&input, INPUTSIZE, inputbuffer);
    option_add("input", "system:console/if:0/event");
    option_add("output", "system:console/if:0/transmit");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

}

