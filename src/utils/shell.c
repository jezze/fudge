#include <fudge.h>
#include <abi.h>

#define INPUTSIZE                       128
#define LINESIZE                        (INPUTSIZE * 2)
#define PROMPTSIZE                      (LINESIZE + 32)

static char inputdata1[INPUTSIZE];
static struct ring input1;
static char inputdata2[INPUTSIZE];
static struct ring input2;
static char line[LINESIZE];
static unsigned int linecount;
static unsigned int sh;
static unsigned int newline = 1;
static unsigned int escaped;
static struct keys keys;
static unsigned int console;

static void print(void *buffer, unsigned int count)
{

    channel_send(0, console, MESSAGE_TYPE_DATA, count, buffer);

}

static void printfmt1(char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    print(buffer, cstring_write_fmt(buffer, MESSAGE_SIZE, 0, fmt, arg1));

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

static void printescape(void *buffer, unsigned int count)
{

    char escape[32];

    escape[0] = 0x1B;

    print(escape, buffer_write(escape, 32, buffer, count, 1) + 1);

}

static void printprompt(char *clear, unsigned int length)
{

    char buffer[PROMPTSIZE];
    unsigned int count = buffer_write(buffer, PROMPTSIZE, clear, length, 0);
    unsigned int rest = ring_count(&input2);

    count += buffer_write(buffer, PROMPTSIZE, "$ ", 2, count);
    count += ring_readcopy(&input1, buffer + count, PROMPTSIZE - count);
    count += ring_readcopy(&input2, buffer + count, PROMPTSIZE - count);

    if (rest)
    {

        buffer[count++] = 0x1B;
        count += cstring_write_fmt(buffer, PROMPTSIZE, count, "[%uD", &rest);

    }

    print(buffer, count);

}

static void cursorleft(unsigned int steps)
{

    if (steps)
    {

        unsigned char num[32];

        printescape(num, cstring_write_fmt(num, 32, 0, "[%uD", &steps));

    }

}

static void cursorright(unsigned int steps)
{

    if (steps)
    {

        unsigned char num[32];

        printescape(num, cstring_write_fmt(num, 32, 0, "[%uC", &steps));

    }

}

static void clearline(void)
{

    printprompt("\033[2K\033[0G", 8);

}

static void clearscreen(void)
{

    printprompt("\033[2J\033[H", 7);

}

static void showprompt(void)
{

    if (!newline)
        print("\n", 1);

    newline = 1;

    clearline();

}

static void insertleft(unsigned int count, void *data)
{

    if (ring_write(&input1, data, count))
    {

        char buffer[INPUTSIZE * 2 + 16];
        unsigned int length = buffer_write(buffer, INPUTSIZE, data, count, 0);
        unsigned int rest = ring_readcopy(&input2, buffer + length, INPUTSIZE);

        if (rest)
        {

            length += rest;
            buffer[length++] = 0x1B;
            length += cstring_write_fmt(buffer, INPUTSIZE * 2 + 16, length, "[%uD", &rest);

        }

        print(buffer, length);

    }

}

static void moveleft(unsigned int steps)
{

    char buffer[INPUTSIZE];

    cursorleft(ring_write_reverse(&input2, buffer, ring_read_reverse(&input1, buffer, steps)));

}

static void moveright(unsigned int steps)
{

    char buffer[INPUTSIZE];

    cursorright(ring_write(&input1, buffer, ring_read(&input2, buffer, steps)));

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

    if (ring_skip_reverse(&input1, steps))
        clearline();

}

static void deleteright(unsigned int steps)
{

    if (ring_skip(&input2, steps))
        clearline();

}

static void deletestart(void)
{

    deleteleft(ring_count(&input1));

}

static void deleteend(void)
{

    deleteright(ring_count(&input2));

}

/* input is a stream of bytes, so a long one goes in as many messages as it takes */
static void sendconsole(void *data, unsigned int count)
{

    unsigned int offset;

    for (offset = 0; offset < count; offset += MESSAGE_CONSOLESIZE)
    {

        struct message_consoledata consoledata;

        consoledata.count = buffer_write(consoledata.data, MESSAGE_CONSOLESIZE, (char *)data + offset, count - offset, 0);

        channel_send(0, sh, MESSAGE_TYPE_CONSOLEDATA, sizeof (struct message_consoledata), &consoledata);

    }

}

static void interrupt(void)
{

    sendconsole("\003", 1);

}

static void run(void)
{

    struct message_pipe pipe;
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

    sh = fs_spawn(1, "initrd:bin/sh");

    if (!sh)
    {

        printfmt1("%s\n", "Could not start sh");
        showprompt();

        return;

    }

    channel_sendoption(0, sh, "pwd", option_getstring("pwd"));
    channel_sendoption(0, sh, "export", "1");
    message_initpipe(&pipe);
    channel_send(0, sh, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
    channel_send(0, sh, MESSAGE_TYPE_DATA, linecount, line);
    channel_send(0, sh, MESSAGE_TYPE_TERM, 0, 0);

}

static void detach(void)
{

    sh = 0;

    showprompt();

}

static void submit(void)
{

    print("\n", 1);

    linecount = ring_read(&input1, line, LINESIZE - 1);
    linecount += ring_read(&input2, line + linecount, LINESIZE - 1 - linecount);
    line[linecount++] = '\n';
    run();

}

static unsigned int runcomplete(char *output, unsigned int size)
{

    char input[INPUTSIZE];

    return system_feed(input, ring_readcopy(&input1, input, INPUTSIZE), output, size, "complete");

}

static void complete(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count = runcomplete(buffer, MESSAGE_SIZE);
    unsigned int length = buffer_findbyte(buffer, count, '\n');

    ring_write(&input1, buffer, length);

    if (length + 1 < count)
    {

        print("\n", 1);
        print(buffer + length + 1, count - length - 1);

    }

}

static void onconsolebyte(unsigned char c)
{

    if (sh)
    {

        switch (c)
        {

        case 0x03:
            interrupt();

            break;

        case 0x1A:
            detach();

            break;

        default:
            sendconsole(&c, 1);

            break;

        }

        return;

    }

    switch (escaped)
    {

    case 1:
        escaped = (c == '[') ? 2 : 0;

        break;

    case 2:
        if (c >= 0x40 && c <= 0x7E)
        {

            switch (c)
            {

            case 'C':
                moveright(1);

                break;

            case 'D':
                moveleft(1);

                break;

            case 'F':
                moveend();

                break;

            case 'H':
                movestart();

                break;

            }

            escaped = 0;

        }

        break;

    default:
        switch (c)
        {

        case 0x01:
            movestart();

            break;

        case 0x02:
            moveleft(1);

            break;

        case 0x05:
            moveend();

            break;

        case 0x06:
            moveright(1);

            break;

        case 0x08:
        case 0x7F:
            deleteleft(1);

            break;

        case 0x09:
            complete();
            clearline();

            break;

        case 0x0A:
        case 0x0D:
            submit();

            break;

        case 0x0B:
            deleteend();

            break;

        case 0x0C:
            clearscreen();

            break;

        case 0x15:
            deletestart();

            break;

        case 0x1B:
            escaped = 1;

            break;

        default:
            if (c >= 0x20)
                insertleft(1, &c);

            break;

        }

        break;

    }

}

static void onconsoledata(struct message *message)
{

    struct message_consoledata *consoledata = message_validate(MESSAGE_TYPE_CONSOLEDATA, message->data, message->length);
    unsigned int i;

    for (i = 0; consoledata && i < consoledata->count; i++)
        onconsolebyte(consoledata->data[i]);

}

static void onkeypress(struct message *message)
{

    struct message_keypress *keypress = message_validate(MESSAGE_TYPE_KEYPRESS, message->data, message->length);

    if (keypress)
    {

        unsigned int id = keys_getcode(&keys, keypress->scancode);

        if (!id)
            return;

        if (sh)
        {

            if (keys.mod & KEYS_MOD_CTRL)
            {

                switch (id)
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

                sendconsole(keys.code.value, keys.code.length);

            }

            return;

        }

        switch (id)
        {

        case KEYS_KEY_BACKSPACE:
            deleteleft(1);

            break;

        case KEYS_KEY_TAB:
            complete();
            clearline();

            break;

        case KEYS_KEY_ENTER:
            submit();

            break;

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

        default:
            insertleft(keys.code.length, keys.code.value);

            break;

        }

    }

}

static void onkeyrelease(struct message *message)
{

    struct message_keyrelease *keyrelease = message_validate(MESSAGE_TYPE_KEYRELEASE, message->data, message->length);

    if (keyrelease)
    {

        keys_getcode(&keys, keyrelease->scancode);

    }

}

static void ondata(struct message *message)
{

    if (message->source == sh)
        printoutput(message->data, message->length);

}

static void onstatus(struct message *message)
{

    struct message_status *status = message_validate(MESSAGE_TYPE_STATUS, message->data, message->length);

    if (message->source == sh)
    {

        sh = 0;

        if (status && status->code == MESSAGE_STATUS_EXIT)
            channel_close(0);
        else
            showprompt();

    }

}

static void onerror(struct message *message)
{

    struct message_error *error = message_validate(MESSAGE_TYPE_ERROR, message->data, message->length);

    print("[ERROR] ", 8);

    if (error)
        printoutput(error->text, cstring_length(error->text));

}

static void onmain(struct message *message)
{

    char *consoleservice = option_getstring("console-service");
    char *keyboardservice = option_getstring("keyboard-service");
    unsigned int keyboard = (cstring_length(keyboardservice)) ? channel_lookup(keyboardservice) : 0;

    console = (cstring_length(consoleservice)) ? channel_lookup(consoleservice) : message->source;

    if (console)
        channel_send(0, console, MESSAGE_TYPE_LINK, 0, 0);

    if (keyboard)
        channel_send(0, keyboard, MESSAGE_TYPE_LINK, 0, 0);

    clearline();
    channel_hold(0);

    if (console)
        channel_send(0, console, MESSAGE_TYPE_UNLINK, 0, 0);

    if (keyboard)
        channel_send(0, keyboard, MESSAGE_TYPE_UNLINK, 0, 0);

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    option_add("console-service", "console:0");
    option_add("keyboard-service", "keyboard:0");
    channel_bind(MESSAGE_TYPE_CONSOLEDATA, onconsoledata);
    channel_bind(MESSAGE_TYPE_KEYPRESS, onkeypress);
    channel_bind(MESSAGE_TYPE_KEYRELEASE, onkeyrelease);
    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_ERROR, onerror);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_STATUS, onstatus);

}

