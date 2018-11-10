#include <fudge.h>
#include <abi.h>

static unsigned char inputbuffer[FUDGE_BSIZE];
static struct ring input;

static void printprompt(void)
{

    file_writeall(FILE_G1, "$ ", 2);

}

static void printnormal(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, "\b\b  \b\b", 6);
    file_writeall(FILE_G1, buffer, count);
    printprompt();

}

static void printcomplete(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, buffer, count);
    printprompt();

}

static unsigned int runcmd(union event_message *imessage, union event_message *omessage, char *command, char *data, unsigned int count, unsigned int session)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, command))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_request(omessage, imessage, EVENT_INIT, id, session);
        event_send(omessage);
        event_request(omessage, imessage, EVENT_DATA, id, session);
        event_adddata(omessage);
        event_appenddata(omessage, count, data);
        event_send(omessage);
        event_request(omessage, imessage, EVENT_STOP, id, session);
        event_send(omessage);

    }

    return id;

}

static unsigned int interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "cd ", 3))
    {

        command[count - 1] = '\0';

        if (file_walk2(FILE_L0, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static unsigned int interpret(union event_message *imessage, union event_message *omessage, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    if (count < 2)
        return 0;

    if (interpretbuiltin(count, command))
        return 0;

    return runcmd(imessage, omessage, "/bin/slang", command, count, 2);

}

static unsigned int complete(union event_message *imessage, union event_message *omessage, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    return runcmd(imessage, omessage, "/bin/complete", command, count, 1);

}

static unsigned int onconsoledata(union event_message *imessage, union event_message *omessage)
{

    struct event_consoledata *consoledata = event_getdata(imessage);

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(imessage, omessage, &input);

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
        interpret(imessage, omessage, &input);
        printprompt();

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

    return 0;

}

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    struct event_data *data = event_getdata(imessage);
    struct job jobs[32];

    switch (imessage->header.session)
    {

    case 0:
        printnormal(data + 1, data->count);

        break;

    case 1:
        printcomplete(data + 1, data->count);

        break;

    case 2:
        job_interpret(jobs, 32, imessage, omessage, data + 1, data->count, 0);

        break;

    }

    return 0;

}

static unsigned int oninit(union event_message *imessage, union event_message *omessage)
{

    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    printprompt();

    return 0;

}

static unsigned int onkill(union event_message *imessage, union event_message *omessage)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    union event_message imessage;
    union event_message omessage;

    if (!file_walk(FILE_G0, FILE_P0, "event"))
        return;

    if (!file_walk(FILE_G1, FILE_P0, "odata"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);
    event_open();

    while (!status)
    {

        event_read(&imessage);

        switch (imessage.header.type)
        {

        case EVENT_CONSOLEDATA:
            status = onconsoledata(&imessage, &omessage);

            break;

        case EVENT_DATA:
            status = ondata(&imessage, &omessage);

            break;

        case EVENT_INIT:
            status = oninit(&imessage, &omessage);

            break;

        case EVENT_KILL:
            status = onkill(&imessage, &omessage);

            break;

        }

    }

    event_close();
    file_close(FILE_G1);
    file_close(FILE_G0);

}

