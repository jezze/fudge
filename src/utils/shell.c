#include <fudge.h>
#include <abi.h>

static unsigned int keymod = KEYMOD_NONE;
static char inputbuffer[FUDGE_BSIZE];
static struct ring input;
static unsigned int idcomplete;
static unsigned int idslang;

static void printprompt(void)
{

    file_writeall(FILE_G1, "$ ", 2);

}

static unsigned int interpretbuiltin(unsigned int count, char *data)
{

    if (memory_match(data, "cd ", 3))
    {

        data[count - 1] = '\0';

        if (file_walk2(FILE_L0, data + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static void interpret(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_BSIZE);

    if (count < 2)
        return;

    if (interpretbuiltin(count, data))
        return;

    channel_request(channel, EVENT_DATA);
    channel_append(channel, count, data);
    channel_place(channel, idslang);

}

static void complete(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_BSIZE);

    channel_request(channel, EVENT_DATA);
    channel_append(channel, count, data);
    channel_place(channel, idcomplete);

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
        printprompt();

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (source == idcomplete)
    {

        file_writeall(FILE_G1, mdata, msize);
        printprompt();

    }

    else if (source == idslang)
    {

        struct job_status status;
        struct job jobs[32];

        status.start = mdata;
        status.end = status.start + msize;

        while (status.start < status.end)
        {

            unsigned int njobs = job_parse(&status, jobs, 32);

            job_run(channel, jobs, njobs);

        }

    }

    else
    {

        file_writeall(FILE_G1, "\b\b  \b\b", 6);
        file_writeall(FILE_G1, mdata, msize);
        printprompt();

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
        printprompt();

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

    if (!file_walk2(FILE_CP, "/bin/complete"))
        return;

    idcomplete = call_spawn(FILE_CP);

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    idslang = call_spawn(FILE_CP);

}

static void onexit(struct channel *channel)
{

    channel_request(channel, EVENT_DONE);
    channel_place(channel, idcomplete);
    channel_place(channel, idslang);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_CONSOLEDATA, onconsoledata);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_KEYPRESS, onkeypress);
    channel_setsignal(&channel, EVENT_KEYRELEASE, onkeyrelease);
    channel_listen2(&channel, oninit, onexit);

}

