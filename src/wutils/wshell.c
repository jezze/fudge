#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128
#define TEXTSIZE                        2048

static char inputdata1[INPUTSIZE];
static struct ring input1;
static char inputdata2[INPUTSIZE];
static struct ring input2;
static char resultdata[TEXTSIZE];
static struct ring result;
static struct job_worker workers[JOBSIZE];
static struct job job;

static void update(void)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    count = ring_readcopy(&result, buffer, BUFFER_SIZE);

    channel_sendfmt2(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= result content \"%w\"\n", buffer, &count);

    count = ring_readcopy(&input1, buffer, BUFFER_SIZE);

    channel_sendfmt2(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= input1 content \"%w\"\n", buffer, &count);

    count = ring_readcopy(&input2, buffer, BUFFER_SIZE);

    channel_sendfmt2(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= input2 content \"%w\"\n", buffer, &count);

}

static void print(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ring_avail(&result))
            ring_skip(&result, ring_find(&result, '\n') + 1);

        if (b[i] != '\r')
            ring_write(&result, &b[i], 1);

    }

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void moveleft(unsigned int steps)
{

    char buffer[INPUTSIZE];

    ring_writereverse(&input2, buffer, ring_readreverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[INPUTSIZE];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

}

static unsigned int runslang(void *obuffer, unsigned int ocount, void *ibuffer, unsigned int icount)
{

    unsigned int id = file_spawn2(FILE_L0, FILE_G8, "/bin/slang");
    unsigned int offset = 0;

    if (id)
    {

        char data[MESSAGE_SIZE];
        unsigned int count;

        channel_listen(id, EVENT_DATA);
        channel_listen(id, EVENT_ERROR);
        channel_listen(id, EVENT_CLOSE);
        channel_sendbuffer(id, EVENT_DATA, icount, ibuffer);
        channel_send(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, EVENT_DATA, data)))
            offset += buffer_write(obuffer, ocount, data, count, offset);

    }

    else
    {

        channel_error("Could not spawn process");

    }

    return offset;

}

static void interpret(void)
{

    char ibuffer[INPUTSIZE];
    unsigned int icount = ring_read(&input1, ibuffer, INPUTSIZE);

    printprompt();
    print(ibuffer, icount);
    update();

    if (icount)
    {

        char obuffer[1000];
        unsigned int ocount = runslang(obuffer, 1000, ibuffer, icount);

        if (ocount)
        {

            job_init(&job, workers, JOBSIZE);
            job_parse(&job, obuffer, ocount);

            if (job_spawn(&job, FILE_L1, FILE_G8))
            {

                struct message message;
                char data[MESSAGE_SIZE];

                job_listen(&job, EVENT_CLOSE);
                job_listen(&job, EVENT_DATA);
                job_listen(&job, EVENT_ERROR);
                job_listen(&job, EVENT_PATH);
                job_pipe(&job, EVENT_DATA);
                job_run(&job);

                while (job_pick(&job, &message, data))
                {

                    switch (message.event)
                    {

                    case EVENT_CLOSE:
                        job_close(&job, message.source);

                        break;

                    case EVENT_ERROR:
                        channel_dispatch(&message, data);

                        break;

                    case EVENT_DATA:
                        print(data, message_datasize(&message));
                        update();

                        break;

                    case EVENT_PATH:
                        if (file_walk(FILE_L0, FILE_G8, data))
                            file_duplicate(FILE_G8, FILE_L0);

                        break;

                    }

                }

            }

            else
            {

                job_killall(&job);

            }

        }

    }

}

static void complete(void)
{

    char path[INPUTSIZE];
    char prefix[INPUTSIZE];

    cstring_writezero(path, INPUTSIZE, 0);
    cstring_writezero(prefix, INPUTSIZE, 0);
    job_init(&job, workers, JOBSIZE);

    job.workers[0].program = "/bin/ls";
    job.count = 1;

    if (ring_count(&input1))
    {

        char buffer[INPUTSIZE];
        unsigned int count = ring_readcopy(&input1, buffer, INPUTSIZE);
        unsigned int lastwordoffset = buffer_lastbyte(buffer, count, ' ');
        unsigned int searchoffset = lastwordoffset + buffer_lastbyte(buffer + lastwordoffset, count - lastwordoffset, '/');
        unsigned int searchcount = count - searchoffset;

        if (lastwordoffset)
        {

            if (searchoffset > lastwordoffset)
            {

                cstring_writezero(path, INPUTSIZE, buffer_write(path, INPUTSIZE, buffer + lastwordoffset, searchoffset - lastwordoffset - 1, 0));

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

            unsigned int prefixcount = cstring_writezero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, (char *)buffer + searchoffset, searchcount, 0));

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

    if (job_spawn(&job, FILE_L1, FILE_G8))
    {

        struct message message;
        char data[MESSAGE_SIZE];
        char outputdata[BUFFER_SIZE];
        struct ring output;

        ring_init(&output, BUFFER_SIZE, outputdata);
        job_listen(&job, EVENT_CLOSE);
        job_listen(&job, EVENT_DATA);
        job_listen(&job, EVENT_ERROR);
        job_pipe(&job, EVENT_DATA);
        job_run(&job);

        while (job_pick(&job, &message, data))
        {

            switch (message.event)
            {

            case EVENT_CLOSE:
                job_close(&job, message.source);

                break;

            case EVENT_ERROR:
                channel_dispatch(&message, data);

                break;

            case EVENT_DATA:
                ring_write(&output, data, message_datasize(&message));

                break;

            }

        }

        if (ring_count(&output))
        {

            if (ring_each(&output, '\n') == ring_count(&output))
            {

                char *outputbuffer = outputdata + cstring_length(prefix);
                unsigned int outputcount = ring_count(&output) - cstring_lengthzero(prefix);

                ring_write(&input1, outputbuffer, outputcount);

            }

            else
            {

                char buffer[INPUTSIZE];

                printprompt();
                print(buffer, ring_readcopy(&input1, buffer, INPUTSIZE));
                print("\n", 1);
                print(outputdata, ring_count(&output));

            }

        }

    }

    else
    {

        job_killall(&job);

    }

}

static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    print("Error: ", 7);
    print(mdata, msize);
    print("\n", 1);
    update();

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_duplicate(FILE_G8, FILE_PW);

    if (!file_walk2(FILE_L0, "system:service/wm"))
        channel_error("window manager service");

    file_notify(FILE_L0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(CHANNEL_DEFAULT, EVENT_WMUNMAP);
    channel_close();

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Shell\"\n"
        "+ layout id \"base\" in \"window\" type \"maximize\" padding \"8\"\n"
        "+ textbox id \"output\" in \"base\"\n"
        "+ text id \"result\" in \"output\" wrap \"char\"\n"
        "+ text id \"prompt\" in \"output\" wrap \"char\" weight \"bold\" content \"$ \"\n"
        "+ text id \"input1\" in \"output\" wrap \"char\"\n"
        "+ text id \"cursor\" in \"output\" wrap \"char\" mode \"inverted\" content \" \"\n"
        "+ text id \"input2\" in \"output\" wrap \"char\"\n";

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data);

}

static void onwmkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress2 *wmkeypress = mdata;

    if (!cstring_match(wmkeypress->pressed, "output"))
        return;

    if (job_count(&job))
    {

        switch (wmkeypress->scancode)
        {

        case 0x2E:
            if (wmkeypress->keymod & KEYMOD_CTRL)
                job_sendfirst(&job, EVENT_TERM, 0, 0);
            else
                job_sendfirst(&job, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

            break;

        default:
            job_sendfirst(&job, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

            break;

        }

    }

    else
    {

        switch (wmkeypress->scancode)
        {

        case 0x0E:
            ring_skipreverse(&input1, 1);

            break;

        case 0x0F:
            ring_move(&input1, &input2);
            complete();

            break;

        case 0x1C:
            ring_move(&input1, &input2);
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);
            interpret();

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

                ring_reset(&result);

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

    }

    update();

}

void init(void)
{

    ring_init(&input1, INPUTSIZE, inputdata1);
    ring_init(&input2, INPUTSIZE, inputdata2);
    ring_init(&result, TEXTSIZE, resultdata);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

