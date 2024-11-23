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

static void update(void)
{

    char buffer[CONTENTSIZE];
    unsigned int count;
    unsigned int cursor;

    count = ring_readcopy(&result, buffer, CONTENTSIZE);

    channel_send_fmt2(0, option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= result content \"%w\"\n", buffer, &count);

    count = ring_readcopy(&input1, buffer, CONTENTSIZE);
    cursor = count;
    count += ring_readcopy(&input2, buffer + count, CONTENTSIZE);

    channel_send_fmt3(0, option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= output cursor \"%u\"\n= input content \"%w\"\n", &cursor, buffer, &count);

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

static void printprompt(void)
{

    print("$ ", 2);

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

static unsigned int runslang(unsigned int ichannel, void *buffer, unsigned int count)
{

    unsigned int target = fs_spawn(ichannel, option_getstring("slang"));

    if (target)
    {

        channel_send(ichannel, target, EVENT_MAIN);
        channel_send_buffer(ichannel, target, EVENT_DATA, count, buffer);
        channel_send(ichannel, target, EVENT_END);

    }

    return target;

}

static void interpretdata(struct message *message, void *buffer)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message_datasize(message));

    if (job_spawn(&job, 0, "initrd:bin"))
    {

        char data[MESSAGE_SIZE];

        job_run(&job, 0, option_getstring("env"), option_getstring("pwd"));

        while (job_pick(&job, 0, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, 0, message->source, message->event, data, message_datasize(message)))
                    print(data, message_datasize(message));

                break;

            }

        }

        update();

    }

    else
    {

        job_killall(&job);
        update();

    }

}

static void interpret(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count = ring_read(&input1, buffer, MESSAGE_SIZE);

    if (count > 1)
    {

        struct message message;
        unsigned int channel = runslang(1, buffer, count);

        printprompt();
        print(buffer, count);
        update();

        while (channel_pollany(1, channel, &message, MESSAGE_SIZE, buffer))
        {

            switch (message.event)
            {

            case EVENT_DATA:
                interpretdata(&message, buffer);

                break;

            case EVENT_DONE:
                return;

            }

        }

    }

    else
    {

        update();

    }

}

static unsigned int createcommand(struct ring *ring, char *ibuffer, char *prefix)
{

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

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace + lastslash, count - lastspace - lastslash, 0));

                return cstring_write_fmt3(ibuffer, INPUTSIZE, "/bin/ls %w | /bin/grep ?prefix %s\n", 0, buffer + lastspace, &lastslash, prefix);

            }

            else
            {

                cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer + lastspace, count - lastspace, 0));

                return cstring_write_fmt1(ibuffer, INPUTSIZE, "/bin/ls | /bin/grep ?prefix %s\n", 0, prefix);

            }

        }

        else
        {

            cstring_write_zero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, buffer, count, 0));

            return cstring_write_fmt1(ibuffer, INPUTSIZE, "/bin/ls /bin | /bin/grep ?prefix %s\n", 0, prefix);

        }

    }

    else
    {

        cstring_write_zero(prefix, INPUTSIZE, 0);

        return cstring_write_fmt0(ibuffer, INPUTSIZE, "/bin/ls\n", 0);

    }

    return 0;

}

static void completedata(struct message *message, char *buffer, unsigned int count, char *prefix)
{

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, buffer, message_datasize(message));

    if (job_spawn(&job, 0, "initrd:bin"))
    {

        char data[MESSAGE_SIZE];
        struct ring output;

        ring_init(&output, INPUTSIZE, buffer);
        job_run(&job, 0, option_getstring("env"), option_getstring("pwd"));

        while (job_pick(&job, 0, message, MESSAGE_SIZE, data))
        {

            switch (message->event)
            {

            case EVENT_DATA:
                if (!job_pipe(&job, 0, message->source, message->event, data, message_datasize(message)))
                    ring_write(&output, data, message_datasize(message));

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

                char tbuffer[INPUTSIZE];

                printprompt();
                print(tbuffer, ring_readcopy(&input1, tbuffer, INPUTSIZE));
                print("\n", 1);
                print(buffer, ring_count(&output));

            }

        }

    }

    else
    {

        job_killall(&job);

    }

}

static void complete(void)
{

    struct message message;
    char prefix[INPUTSIZE];
    char buffer[MESSAGE_SIZE];
    unsigned int count = createcommand(&input1, buffer, prefix);
    unsigned int channel = runslang(2, buffer, count);

    while (channel_pollany(2, channel, &message, MESSAGE_SIZE, buffer))
    {

        switch (message.event)
        {

        case EVENT_DATA:
            completedata(&message, buffer, count, prefix);

            break;

        case EVENT_DONE:
            return;

        }

    }

}

static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    print("[ERROR] ", 8);
    print(mdata, msize);
    update();

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    lookup("wm-service");
    channel_send(0, option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process(0));

    channel_send(0, option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Shell\"\n"
        "  + layout id \"base\" in \"window\" flow \"horizontal-stretch\" padding \"1\"\n"
        "    + textbox id \"output\" in \"base\" overflow \"vscroll\" mode \"readonly\" span \"1\" cursor \"0\"\n"
        "      + text id \"result\" in \"output\" wrap \"char\"\n"
        "      + text id \"prompt\" in \"output\" wrap \"char\" weight \"bold\" content \"$ \"\n"
        "      + text id \"input\" in \"output\" wrap \"char\" content \"\"\n";

    channel_send_fmt0(0, option_getdecimal("wm-service"), EVENT_WMRENDERDATA, data);

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
                job_sendfirst(&job, 0, EVENT_TERM, 0, 0);

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

            case KEYS_KEY_U:
                ring_reset(&input1);

                break;

            case KEYS_KEY_K:
                ring_reset(&input2);

                break;

            case KEYS_KEY_L:
                ring_reset(&result);

                break;

            }

        }

        else
        {

            switch (wmkeypress->id)
            {

            case KEYS_KEY_BACKSPACE:
                ring_skip_reverse(&input1, 1);

                break;

            case KEYS_KEY_TAB:
                ring_move(&input1, &input2);
                complete();

                break;

            case KEYS_KEY_ENTER:
                ring_move(&input1, &input2);
                ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);
                interpret();

                break;

            case KEYS_KEY_HOME:
                moveleft(ring_count(&input1));

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
                moveright(ring_count(&input2));

                break;

            case KEYS_KEY_CURSORDOWN:
                if (wmkeypress->keymod & KEYS_MOD_SHIFT)
                {
                    /*content.offset++;*/
                }

                break;

            default:
                ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

                break;

            }

        }

    }

    update();

}

void init(void)
{

    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    ring_init(&result, RESULTSIZE, resultdata);
    option_add("wm-service", "");
    option_add("slang", "initrd:bin/slang");
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

