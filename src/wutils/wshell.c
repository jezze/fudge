#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128
#define RESULTSIZE                      2048
#define CONTENTSIZE                     1200

static char inputdata1[INPUTSIZE];
static struct ring input1;
static char inputdata2[INPUTSIZE];
static struct ring input2;
static char resultdata[RESULTSIZE];
static struct ring result;
static struct job_worker workers[JOBSIZE];
static struct job job;

static void update(unsigned int wm)
{

    char buffer[CONTENTSIZE];
    unsigned int count;
    unsigned int cursor;

    count = ring_readcopy(&result, buffer, CONTENTSIZE);

    channel_send_fmt2(0, wm, EVENT_WMRENDERDATA, "= result label \"%w\"\n", buffer, &count);

    count = ring_readcopy(&input1, buffer, CONTENTSIZE);
    cursor = count;
    count += ring_readcopy(&input2, buffer + count, CONTENTSIZE);

    channel_send_fmt3(0, wm, EVENT_WMRENDERDATA, "= output cursor \"%u\"\n= input label \"%w\"\n", &cursor, buffer, &count);

}

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

static void printring(struct ring *ring)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count = ring_readcopy(ring, buffer, MESSAGE_SIZE);

    if (count)
        print(buffer, count);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void clearline(void)
{

    ring_reset(&result);

}

static void insertleft(unsigned int count, void *data)
{

    ring_write(&input1, data, count);

}

static void insertright(unsigned int count, void *data)
{

    ring_write(&input2, data, count);

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

static unsigned int runslang(unsigned int ichannel, void *buffer, unsigned int count)
{

    unsigned int target = fs_spawn(ichannel, option_getstring("slang"));

    if (target)
    {

        channel_send(ichannel, target, EVENT_MAIN, 0, 0);
        channel_send(ichannel, target, EVENT_DATA, count, buffer);
        channel_send(ichannel, target, EVENT_TERM, 0, 0);

    }

    return target;

}

static void interpretdata(unsigned int ichannel, struct message *message, void *buffer)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message->length);

    if (job_exec(&job, ichannel, "initrd:bin", option_getstring("pwd")))
    {

        char data[MESSAGE_SIZE];

        while (job_pick(&job, ichannel, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, ichannel, message->source, message->event, data, message->length))
                    print(data, message->length);

                break;

            }

        }

    }

}

static void interpret(unsigned int wm)
{

    unsigned char buffer[MESSAGE_SIZE];
    unsigned int count = 0;

    count += ring_read(&input1, buffer + count, MESSAGE_SIZE - count);
    count += ring_read(&input2, buffer + count, MESSAGE_SIZE - count);

    if (count > 1)
    {

        unsigned int channel = runslang(1, buffer, count);

        printprompt();
        print(buffer, count);
        update(wm);

        if (channel)
        {

            struct message message;

            while (channel_poll(1, channel, EVENT_ALL, &message, MESSAGE_SIZE, buffer))
            {

                switch (message.event)
                {

                case EVENT_DATA:
                    interpretdata(0, &message, buffer);

                    break;

                case EVENT_DONE:
                    return;

                }

            }

        }

    }

}

static unsigned int createcommand(char *ibuffer, char *prefix)
{

    unsigned char buffer[INPUTSIZE];
    unsigned int count = 0;

    count += ring_readcopy(&input1, buffer + count, INPUTSIZE - count);
    count += ring_readcopy(&input2, buffer + count, INPUTSIZE - count);

    if (count)
    {

        unsigned int lastspace = buffer_lastbyte(buffer, count, ' ');

        if (lastspace)
        {

            unsigned int lastslash = buffer_lastbyte(buffer + lastspace, count - lastspace, '/');

            if (lastslash)
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace + lastslash, count - lastspace - lastslash, 0));

                return cstring_write_fmt3(ibuffer, INPUTSIZE, 0, "/bin/ls %w | /bin/grep ?prefix %s\n", buffer + lastspace, &lastslash, prefix);

            }

            else
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace, count - lastspace, 0));

                return cstring_write_fmt1(ibuffer, INPUTSIZE, 0, "/bin/ls | /bin/grep ?prefix %s\n", prefix);

            }

        }

        else
        {

            cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer, count, 0));

            return cstring_write_fmt1(ibuffer, INPUTSIZE, 0, "/bin/ls /bin | /bin/grep ?prefix %s\n", prefix);

        }

    }

    else
    {

        cstring_write_zero(prefix, INPUTSIZE, 0);

        return cstring_write_fmt0(ibuffer, INPUTSIZE, 0, "/bin/ls\n");

    }

    return 0;

}

static void completedata(unsigned int ichannel, struct message *message, char *buffer, unsigned int count, char *prefix)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message->length);

    if (job_exec(&job, ichannel, "initrd:bin", option_getstring("pwd")))
    {

        char data[MESSAGE_SIZE];
        struct ring output;

        ring_init(&output, INPUTSIZE, buffer);

        while (job_pick(&job, ichannel, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, ichannel, message->source, message->event, data, message->length))
                    ring_write(&output, data, message->length);

                break;

            }

        }

        if (ring_count(&output))
        {

            if (ring_each(&output, '\n') == ring_count(&output))
            {

                char *outputbuffer = buffer + cstring_length(prefix);
                unsigned int outputcount = ring_count(&output) - cstring_length_zero(prefix);

                ring_write(&input1, outputbuffer, outputcount);

            }

            else
            {

                printprompt();
                printring(&input1);
                printring(&input2);
                print("\n", 1);
                printring(&output);

            }

        }

    }

}

static void complete(void)
{

    char prefix[INPUTSIZE];
    char buffer[MESSAGE_SIZE];
    unsigned int count = createcommand(buffer, prefix);

    if (count)
    {

        unsigned int channel = runslang(2, buffer, count);

        if (channel)
        {

            struct message message;

            while (channel_poll(2, channel, EVENT_ALL, &message, MESSAGE_SIZE, buffer))
            {

                switch (message.event)
                {

                case EVENT_DATA:
                    completedata(0, &message, buffer, count, prefix);

                    break;

                case EVENT_DONE:
                    return;

                }

            }

        }

    }

}

static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        print("[ERROR] ", 8);
        print(mdata, msize);
        update(wm);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(0, wm, EVENT_WMMAP, 0, 0);

        while (channel_process(0) != EVENT_WMCLOSE);

        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" label \"Shell\"\n"
        "  + textbox id \"output\" in \"window\" flow \"vertical\" overflow \"vscroll\" mode \"readonly\" span \"1\" cursor \"0\"\n"
        "    + text id \"result\" in \"output\" display \"inline\" wrap \"char\"\n"
        "    + text id \"prompt\" in \"output\" display \"inline\" wrap \"char\" weight \"bold\" label \"$ \"\n"
        "    + text id \"input\" in \"output\" display \"inline\" wrap \"char\" label \"\"\n";

    channel_send_fmt0(0, source, EVENT_WMRENDERDATA, data);

}

static void onwmkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress *wmkeypress = mdata;

    if (job_count(&job))
    {

        if (wmkeypress->keymod & KEYS_MOD_CTRL)
        {

            switch (wmkeypress->id)
            {

            case KEYS_KEY_C:
                job_sendfirst(&job, 0, EVENT_KILL, 0, 0);

                break;

            }

        }

        else
        {

            job_sendfirst(&job, 0, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

        }

    }

    else
    {

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
                clearline();

                break;

            case KEYS_KEY_U:
                deletestart();

                break;

            }

        }

        else
        {

            switch (wmkeypress->id)
            {

            case KEYS_KEY_BACKSPACE:
                deleteleft(1);

                break;

            case KEYS_KEY_TAB:
                complete();

                break;

            case KEYS_KEY_ENTER:
                insertright(wmkeypress->length, &wmkeypress->unicode);
                interpret(source);

                break;

            case KEYS_KEY_HOME:
                movestart();

                break;

            case KEYS_KEY_CURSORUP:
                if (wmkeypress->keymod & KEYS_MOD_SHIFT)
                {
                    /*content.offset--;*/
                }

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

            case KEYS_KEY_CURSORDOWN:
                if (wmkeypress->keymod & KEYS_MOD_SHIFT)
                {
                    /*content.offset++;*/
                }

                break;

            default:
                insertleft(wmkeypress->length, &wmkeypress->unicode);

                break;

            }

        }

    }

    update(source);

}

void init(void)
{

    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    ring_init(&result, RESULTSIZE, resultdata);
    option_add("wm-service", "wm");
    option_add("slang", "initrd:bin/slang");
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

    while (channel_process(0));

}

