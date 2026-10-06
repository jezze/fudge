#include <fudge.h>
#include <abi.h>

#define INPUTSIZE                       128
#define RESULTSIZE                      2048
#define CONTENTSIZE                     1200
#define LINESIZE                        (INPUTSIZE * 2)

static char inputdata1[INPUTSIZE];
static struct ring input1;
static char inputdata2[INPUTSIZE];
static struct ring input2;
static char resultdata[RESULTSIZE];
static struct ring result;
static char line[LINESIZE];
static unsigned int linecount;
static unsigned int sh;
static unsigned int newline = 1;

static void print(void *buffer, unsigned int count)
{

    ring_overwrite(&result, buffer, count);

    if (ring_count(&result) >= CONTENTSIZE)
    {

        unsigned int nl;

        ring_skip(&result, ring_count(&result) - CONTENTSIZE + 1);

        nl = ring_find(&result, '\n');

        if (nl)
            ring_skip(&result, nl + 1);

    }

}

static void printfmt1(char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    print(buffer, cstring_write_fmt1(buffer, MESSAGE_SIZE, 0, fmt, arg1));

}

static void printoutput(void *buffer, unsigned int count)
{

    if (count)
    {

        char *data = buffer;

        print(buffer, count);

        newline = (data[count - 1] == '\n');

    }

}

static void printring(struct ring *ring)
{

    char buffer[INPUTSIZE];
    unsigned int count = ring_readcopy(ring, buffer, INPUTSIZE);

    if (count)
        print(buffer, count);

}

static unsigned int fitlabel(char *data, unsigned int count, unsigned int size)
{

    unsigned int total = 0;
    unsigned int start = count;

    while (start > 0)
    {

        unsigned int width = (data[start - 1] == '"') ? 4 : 1;

        if (total + width > size)
            break;

        total += width;
        start--;

    }

    return start;

}

static unsigned int writelabel(char *buffer, unsigned int size, unsigned int offset, char *id, char *data, unsigned int count)
{

    unsigned int start;
    unsigned int i;

    offset += cstring_write_fmt1(buffer, size, offset, "= %s label \"", id);
    start = (size > offset + 2) ? fitlabel(data, count, size - offset - 2) : count;

    for (i = start; i < count; i++)
    {

        if (data[i] == '"')
            offset += buffer_write(buffer, size, "\"\\\"\"", 4, offset);
        else
            offset += buffer_write(buffer, size, &data[i], 1, offset);

    }

    offset += buffer_write(buffer, size, "\"\n", 2, offset);

    return offset;

}

static void update(void)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char buffer[MESSAGE_SIZE];
        char content[CONTENTSIZE];
        unsigned int offset = 0;
        unsigned int count;
        unsigned int cursor;

        count = ring_readcopy(&input1, content, CONTENTSIZE);
        cursor = count;
        count += ring_readcopy(&input2, content + count, CONTENTSIZE - count);
        offset += cstring_write_fmt1(buffer, MESSAGE_SIZE, offset, "= output cursor \"%u\"\n", &cursor);
        offset = writelabel(buffer, MESSAGE_SIZE, offset, "input", content, count);
        offset = writelabel(buffer, MESSAGE_SIZE, offset, "prompt", "$ ", (sh) ? 0 : 2);
        count = ring_readcopy(&result, content, CONTENTSIZE);
        offset = writelabel(buffer, MESSAGE_SIZE, offset, "result", content, count);

        channel_send(0, wm, EVENT_WMRENDERDATA, offset, buffer);

    }

}

static void showprompt(void)
{

    if (!newline)
        print("\n", 1);

    newline = 1;

    update();

}

static void clearresult(void)
{

    ring_reset(&result);

    newline = 1;

}

static void insertleft(unsigned int count, void *data)
{

    ring_write(&input1, data, count);

}

static void moveleft(unsigned int steps)
{

    char buffer[INPUTSIZE];

    ring_write_reverse(&input2, buffer, ring_read_reverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[INPUTSIZE];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

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

    ring_skip_reverse(&input1, steps);

}

static void deleteright(unsigned int steps)
{

    ring_skip(&input2, steps);

}

static void deletestart(void)
{

    ring_reset(&input1);

}

static void deleteend(void)
{

    ring_reset(&input2);

}

static void interrupt(void)
{

    channel_send(0, sh, EVENT_CONSOLEDATA, 1, "\003");

}

static void run(void)
{

    unsigned int i;

    for (i = 0; i < linecount; i++)
    {

        if (line[i] != ' ' && line[i] != '\n')
            break;

    }

    if (i == linecount)
    {

        showprompt();

        return;

    }

    sh = fs_spawn(1, 0, "initrd:bin/sh");

    if (!sh)
    {

        printfmt1("%s\n", "Could not start sh");
        showprompt();

        return;

    }

    channel_send_fmt1(0, sh, EVENT_OPTION, "pwd=%s&export=1\n", option_getstring("pwd"));
    channel_send(0, sh, EVENT_MAIN, 0, 0);
    channel_send(0, sh, EVENT_DATA, linecount, line);
    channel_send(0, sh, EVENT_TERM, 0, 0);
    update();

}

static void detach(void)
{

    sh = 0;

    showprompt();

}

static void submit(void)
{

    linecount = ring_read(&input1, line, LINESIZE - 1);
    linecount += ring_read(&input2, line + linecount, LINESIZE - 1 - linecount);
    line[linecount++] = '\n';

    print("$ ", 2);
    print(line, linecount);

    newline = 1;

    run();

}

static unsigned int runcomplete(char *output, unsigned int size)
{

    unsigned int target = fs_spawn(1, 1, "initrd:bin/complete");
    unsigned int count = 0;

    if (target)
    {

        char input[INPUTSIZE];
        struct message message;

        channel_send_fmt1(1, target, EVENT_OPTION, "pwd=%s\n", option_getstring("pwd"));
        channel_send(1, target, EVENT_MAIN, 0, 0);
        channel_send(1, target, EVENT_DATA, ring_readcopy(&input1, input, INPUTSIZE), input);
        channel_send(1, target, EVENT_TERM, 0, 0);

        while (channel_poll(1, target, EVENT_DATA, &message))
            count += buffer_write(output, size, message.data, message.length, count);

    }

    return count;

}

static void complete(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count = runcomplete(buffer, MESSAGE_SIZE);
    unsigned int length = buffer_findbyte(buffer, count, '\n');

    ring_write(&input1, buffer, length);

    if (length + 1 < count)
    {

        print("$ ", 2);
        printring(&input1);
        printring(&input2);
        print("\n", 1);
        printoutput(buffer + length + 1, count - length - 1);

    }

}

static void ondata(struct message *message)
{

    if (message->source == sh)
    {

        printoutput(message->data, message->length);
        update();

    }

}

static void onexit(struct message *message)
{

    if (message->source == sh)
    {

        sh = 0;

        showprompt();

    }

}

static void onerror(struct message *message)
{

    print("[ERROR] ", 8);
    printoutput(message->data, message->length);
    update();

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(0, wm, EVENT_WMMAP, 0, 0);
        channel_hold(0);
        interrupt();
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wshell.alfi";

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    update();

}

static void onwmkeypress(struct message *message)
{

    struct event_wmkeypress *wmkeypress = message->data;

    if (sh)
    {

        if (wmkeypress->keymod & KEYS_MOD_CTRL)
        {

            switch (wmkeypress->id)
            {

            case KEYS_KEY_C:
                interrupt();

                break;

            case KEYS_KEY_Z:
                detach();

                break;

            }

        }

        else
        {

            channel_send(0, sh, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

        }

        return;

    }

    if (wmkeypress->keymod & KEYS_MOD_CTRL)
    {

        switch (wmkeypress->id)
        {

        case KEYS_KEY_A:
            movestart();

            break;

        case KEYS_KEY_B:
            moveleft(1);

            break;

        case KEYS_KEY_D:
            deleteright(1);

            break;

        case KEYS_KEY_E:
            moveend();

            break;

        case KEYS_KEY_F:
            moveright(1);

            break;

        case KEYS_KEY_H:
            deleteleft(1);

            break;

        case KEYS_KEY_K:
            deleteend();

            break;

        case KEYS_KEY_L:
            clearresult();

            break;

        case KEYS_KEY_U:
            deletestart();

            break;

        }

        update();

        return;

    }

    switch (wmkeypress->id)
    {

    case KEYS_KEY_BACKSPACE:
        deleteleft(1);

        break;

    case KEYS_KEY_DELETE:
        deleteright(1);

        break;

    case KEYS_KEY_TAB:
        complete();

        break;

    case KEYS_KEY_ENTER:
        submit();

        return;

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

    case KEYS_KEY_CURSORUP:
    case KEYS_KEY_CURSORDOWN:
        break;

    default:
        insertleft(wmkeypress->length, &wmkeypress->unicode);

        break;

    }

    update();

}

void init(void)
{

    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    ring_init(&result, RESULTSIZE, resultdata);
    option_add("wm-service", "wm");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

