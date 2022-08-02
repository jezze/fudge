#include <fudge.h>
#include <abi.h>

static char outputdata[BUFFER_SIZE];
static struct ring output;
static char inputdata1[128];
static struct ring input1;
static char inputdata2[128];
static struct ring input2;
static char textdata[1024];
static struct ring text;

static void update(void)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += buffer_write(buffer, BUFFER_SIZE, "= input1 content \"", 18, count);
    count += buffer_write(buffer, BUFFER_SIZE, "hej", 3, count);
    count += buffer_write(buffer, BUFFER_SIZE, "\"\n", 2, count);

    file_notify(FILE_G0, EVENT_WMRENDERDATA, count, buffer);

}

static void print(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ring_avail(&text))
            ring_skip(&text, ring_find(&text, '\n') + 1);

        if (b[i] != '\r')
            ring_write(&text, &b[i], 1);

    }

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void moveleft(unsigned int steps)
{

    char buffer[128];

    ring_writereverse(&input2, buffer, ring_readreverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[128];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Shell\"\n"
        "+ container id \"base\" in \"window\" layout \"maximize\" padding \"16\"\n"
        "+ textbox id \"output\" in \"base\"\n"
        "+ text id \"result\" in \"output\" wrap \"none\"\n"
        "+ text id \"prompt\" in \"output\" wrap \"none\" weight \"bold\" content \"$ \"\n"
        "+ text id \"input1\" in \"output\" wrap \"none\"\n"
        "+ text id \"cursor\" in \"output\" wrap \"none\" mode \"inverted\" content \" \"\n"
        "+ text id \"input2\" in \"output\" wrap \"none\"\n";

    file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data), data);

}

static void onwmkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress2 *wmkeypress = mdata;

    if (!cstring_match(wmkeypress->pressed, "output"))
        return;

    switch (wmkeypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);

        break;

    case 0x0F:
        ring_move(&input1, &input2);
        /* complete(&input1); */

        break;

    case 0x1C:
        ring_move(&input1, &input2);
        ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);
        /* interpret(&input1); */

        break;

    case 0x16:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            ring_reset(&input1);
        else
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    case 0x25:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            ring_reset(&input2);
        else
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    case 0x26:
        if (wmkeypress->keymod & KEYMOD_CTRL)
        {

            ring_reset(&text);
            printprompt();

        }

        else
        {

            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        }

        break;

    case 0x47:
        moveleft(ring_count(&input1));

        break;

    case 0x48:
        if (wmkeypress->keymod & KEYMOD_SHIFT)
        {
            /*content.offset--;*/
        }

        break;

    case 0x4B:
        moveleft(1);

        break;

    case 0x4D:
        moveright(1);

        break;

    case 0x4F:
        moveright(ring_count(&input2));

        break;

    case 0x50:
        if (wmkeypress->keymod & KEYMOD_SHIFT)
        {
            /*content.offset++;*/
        }

        break;

    default:
        ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    }

    update();

}

void init(void)
{

    ring_init(&output, BUFFER_SIZE, outputdata);
    ring_init(&input1, 128, inputdata1);
    ring_init(&input2, 128, inputdata2);
    ring_init(&text, 1024, textdata);

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

