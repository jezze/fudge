#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128

static char inputbuffer[INPUTSIZE];
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

static void handleinput(struct job *job, void *mdata)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case 0x03:
        job_sendfirst(job, EVENT_TERM, 0, 0);

        break;

    default:
        job_sendfirst(job, EVENT_CONSOLEDATA, 1, &consoledata->data);

        break;

    }

}

static void interpretspawn(struct job *job, unsigned int count, void *buffer)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        struct message message;

        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_ERROR);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendbufferto(id, EVENT_DATA, count, buffer);
        channel_sendto(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, message.data.buffer, MESSAGE_SIZE)))
            job_parse(job, message.data.buffer, count);

    }

}

static void interpretprocess(struct job *job)
{

    struct message message;

    while (job_pick(job, &message))
    {

        switch (message.header.event)
        {

        case EVENT_CLOSE:
            job_close(job, message.header.source);

            break;

        case EVENT_CONSOLEDATA:
            handleinput(job, message.data.buffer);

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

static void interpret(struct ring *ring)
{

    char buffer[INPUTSIZE];
    unsigned int count = ring_read(ring, buffer, INPUTSIZE);

    if (count)
    {

        struct job_worker workers[JOBSIZE];
        struct job job;

        job_init(&job, workers, JOBSIZE);
        interpretspawn(&job, count, buffer);
        job_spawn(&job);
        job_pipe(&job);
        job_run(&job);
        interpretprocess(&job);

    }

    printprompt();

}

static char prefix[INPUTSIZE];
static unsigned int prefixcount;

static void completespawn(struct job *job, unsigned int count, void *buffer)
{

    job->workers[0].program = "/bin/ls";
    job->count = 1;

    if (count)
    {

        unsigned int lastwordoffset;
        unsigned int lastwordcount;
        unsigned int searchoffset;
        unsigned int searchcount;
        char search[INPUTSIZE];

        lastwordoffset = buffer_lastbyte(buffer, count, ' ');
        lastwordcount = count - lastwordoffset;
        searchoffset = lastwordoffset + buffer_lastbyte((char *)buffer + lastwordoffset, lastwordcount, '/');
        searchcount = count - searchoffset;

        buffer_copy(search, (char *)buffer + searchoffset, searchcount);

        prefixcount = 0;
        prefixcount += buffer_write(prefix, INPUTSIZE, search, searchcount, prefixcount);
        prefixcount += cstring_writez(prefix, INPUTSIZE, "", prefixcount);

        if (lastwordoffset > 0)
        {

            job->workers[1].program = "/bin/grep";
            job->workers[1].options[0].key = "prefix";
            job->workers[1].options[0].value = prefix;
            job->workers[1].noptions = 1;
            job->count = 2;

        }

        else
        {

            job->workers[0].paths[0] = "/bin";
            job->workers[0].npaths = 1;
            job->workers[1].program = "/bin/grep";
            job->workers[1].options[0].key = "prefix";
            job->workers[1].options[0].value = prefix;
            job->workers[1].noptions = 1;
            job->count = 2;

        }

    }

}

static void completeprocess(struct job *job)
{

    struct message message;

    while (job_pick(job, &message))
    {

        switch (message.header.event)
        {

        case EVENT_CLOSE:
            job_close(job, message.header.source);

            break;

        case EVENT_DATA:
            if (buffer_firstbyte(message.data.buffer, message_datasize(&message.header), '\n') == message_datasize(&message.header))
            {

                ring_write(&input, message.data.buffer + prefixcount - 1, message_datasize(&message.header) - prefixcount);
                print(message.data.buffer + prefixcount - 1, message_datasize(&message.header) - prefixcount);

            }

            else
            {

                /* this does not happen right now */

                print("\n", 1);
                print(message.data.buffer, message_datasize(&message.header));

            }

            break;

        }

    }

}

static void complete(struct ring *ring)
{

    char buffer[INPUTSIZE];
    unsigned int count = ring_readcopy(ring, buffer, INPUTSIZE);
    struct job_worker workers[JOBSIZE];
    struct job job;

    job_init(&job, workers, JOBSIZE);
    completespawn(&job, count, buffer);
    job_spawn(&job);
    job_pipe(&job);
    job_run(&job);
    completeprocess(&job);

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\f':
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
 
static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    print("Error: ", 7);
    print(mdata, msize);
    print("\n", 1);

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
    char *value = key + cstring_lengthz(key);

    if (cstring_match(key, "console"))
    {

        if (file_walk2(FILE_L0, value))
        {

            file_walk(FILE_G0, FILE_L0, "event");
            file_walk(FILE_G1, FILE_L0, "transmit");

        }

    }

    if (cstring_match(key, "keyboard"))
    {

        if (file_walk2(FILE_L0, value))
        {

            file_walk(FILE_G2, FILE_L0, "event");

        }

    }

}

void init(void)
{

    ring_init(&input, INPUTSIZE, inputbuffer);
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

