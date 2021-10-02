#include <fudge.h>
#include <abi.h>

static char inputbuffer[BUFFER_SIZE];
static struct ring input;
static unsigned int keymod = KEYMOD_NONE;

static void print(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, buffer, count);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void check(void *mdata, struct job *jobs, unsigned int njobs)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case 0x03:
        job_send(jobs, njobs, EVENT_TERM, 0, 0);

        break;

    default:
        job_send(jobs, njobs, EVENT_CONSOLEDATA, 1, &consoledata->data);

        break;

    }

}

static void runcommand(unsigned int count, void *buffer)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        struct message message;
        struct job jobs[32];
        unsigned int njobs = 0;
        unsigned int tasks;

        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendbufferto(id, EVENT_DATA, count, buffer);
        channel_sendto(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, message.data.buffer, MESSAGE_SIZE)))
        {

            unsigned int n = job_parse(jobs, 32, message.data.buffer, count);

            njobs = job_spawn(jobs, n);

        }

        job_pipe(jobs, njobs);

        tasks = job_run(jobs, njobs);

        while (tasks && channel_pick(&message))
        {

            switch (message.header.event)
            {

            case EVENT_CLOSE:
                tasks = job_close(message.header.source, jobs, njobs);

                break;

            case EVENT_CONSOLEDATA:
                check(message.data.buffer, jobs, njobs);

                break;

            default:
                channel_dispatch(&message);

                break;

            }

        }

    }

}

static void interpret(struct ring *ring)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = ring_read(ring, buffer, BUFFER_SIZE);

    if (count >= 2)
        runcommand(count, buffer);

    printprompt();

}

static void complete(struct ring *ring)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = ring_read(ring, buffer, BUFFER_SIZE);
    unsigned int id = file_spawn("/bin/complete");

    if (id)
    {

        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendbufferto(id, EVENT_DATA, count, buffer);
        channel_sendto(id, EVENT_MAIN);

        while (channel_readfrom(id, buffer, BUFFER_SIZE));

    }

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(&input);

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
        interpret(&input);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        print(&consoledata->data, 1);

        break;

    }

}

static void onkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keypress *keypress = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, keymod);

    keymod = keymap_modkey(keypress->scancode, keymod);

    switch (keypress->scancode)
    {

    case 0x0E:
        if (!ring_skipreverse(&input, 1))
            break;

        print("\b \b", 3);

        break;

    case 0x0F:
        complete(&input);

        break;

    case 0x1C:
        print(keycode->value, keycode->length);
        ring_write(&input, keycode->value, keycode->length);
        interpret(&input);

        break;

    default:
        ring_write(&input, keycode->value, keycode->length);
        print(keycode->value, keycode->length);

        break;

    }

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keymod = keymap_modkey(keyrelease->scancode, keymod);

}
 
static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    print(mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    printprompt();
    file_link(FILE_G0);
    file_link(FILE_G2);

    while (channel_process());

    file_unlink(FILE_G2);
    file_unlink(FILE_G0);

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "console"))
    {

        if (file_walk2(FILE_L0, value))
        {

            file_walk(FILE_G0, FILE_L0, "event");
            file_walk(FILE_G1, FILE_L0, "transmit");

        }

    }

    if (ascii_match(key, "keyboard"))
    {

        if (file_walk2(FILE_L0, value))
        {

            file_walk(FILE_G2, FILE_L0, "event");

        }

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_CW, mdata))
        file_duplicate(FILE_CW, FILE_L0);

}

void init(void)
{

    ring_init(&input, BUFFER_SIZE, inputbuffer);
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_PATH, onpath);

}

