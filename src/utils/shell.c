#include <fudge.h>
#include <abi.h>

#define MODE_NORMAL                     0
#define MODE_WAITING                    1

static unsigned int keymod = KEYMOD_NONE;
static char inputbuffer[FUDGE_BSIZE];
static struct ring input;
static unsigned int idcomplete;
static unsigned int idslang;
static unsigned int mode = MODE_NORMAL;

static void printprompt(void)
{

    switch (mode)
    {

    case MODE_NORMAL:
        file_writeall(FILE_G1, "$ ", 2);

        break;

    }

}

static unsigned int interpretbuiltin(unsigned int count, char *data)
{

    if (count < 2)
    {

        printprompt();

        return 1;

    }

    if (memory_match(data, "cd ", 3))
    {

        data[count - 1] = '\0';

        if (file_walk2(FILE_L0, data + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        printprompt();

        return 1;

    }

    return 0;

}

static void interpret(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_MSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_MSIZE);

    if (mode == MODE_NORMAL)
    {

        if (interpretbuiltin(count, data))
            return;

        channel_place(channel, idslang, EVENT_DATA, count, data);

    }

}

static void complete(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_MSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_MSIZE);

    if (mode == MODE_NORMAL)
        channel_place(channel, idcomplete, EVENT_DATA, count, data);

}

static void completedata(struct channel *channel, void *mdata, unsigned int msize)
{

    file_writeall(FILE_G1, mdata, msize);
    printprompt();

}

static void slangdata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct job_status status;
    struct job jobs[32];
    unsigned int njobs = 0;
    unsigned int nids = 0;
    unsigned int i;

    status.start = mdata;
    status.end = status.start + msize;

    mode = MODE_WAITING;

    while (status.start < status.end)
    {

        njobs = job_parse(&status, jobs, 32);

        job_run(channel, jobs, njobs);

    }

    for (i = 0; i < njobs; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            nids++;

    }

    if (nids)
    {

        struct message_header header;
        char data[FUDGE_MSIZE];

        while (channel_poll(channel, &header, data, EVENT_CLOSE))
        {

            if (--nids == 0)
                break;

        }

    }

    mode = MODE_NORMAL;

    printprompt();

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(channel, &input);

        break;

    case '\b':
    case 0x7F:
        if (!ring_skipreverse(&input, 1))
            break;

        file_writeall(FILE_G1, "\b \b", 3);

        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        file_writeall(FILE_G1, &consoledata->data, 1);
        ring_write(&input, &consoledata->data, 1);
        interpret(channel, &input);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    printprompt();

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    switch (mode)
    {

    case MODE_NORMAL:
        if (source == idcomplete)
            completedata(channel, mdata, msize);
        else if (source == idslang)
            slangdata(channel, mdata, msize);
        else
            file_writeall(FILE_G1, mdata, msize);

        break;

    case MODE_WAITING:
        file_writeall(FILE_G1, mdata, msize);

        break;

    }

}

static unsigned int desc = FILE_G0;

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(desc, mdata))
        return;

    file_open(desc);

    desc++;

}

static void onkeypress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
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

        file_writeall(FILE_G1, "\b \b", 3);

        break;

    case 0x0F:
        complete(channel, &input);

        break;

    case 0x1C:
        file_writeall(FILE_G1, keycode->value, keycode->length);
        ring_write(&input, keycode->value, keycode->length);
        interpret(channel, &input);

        break;

    default:
        ring_write(&input, keycode->value, keycode->length);
        file_writeall(FILE_G1, keycode->value, keycode->length);

        break;

    }

}

static void onkeyrelease(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keymod = keymap_modkey(keyrelease->scancode, keymod);

}

static void oninit(struct channel *channel)
{

    ring_init(&input, FUDGE_BSIZE, inputbuffer);

    idcomplete = file_spawn(FILE_CP, "/bin/complete");

    if (!idcomplete)
        return;

    idslang = file_spawn(FILE_CP, "/bin/slang");

    if (!idslang)
        return;

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_CONSOLEDATA, onconsoledata);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_KEYPRESS, onkeypress);
    channel_setsignal(&channel, EVENT_KEYRELEASE, onkeyrelease);
    channel_listen(&channel, oninit, 0);

}

