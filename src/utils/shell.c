#include <abi.h>
#include <fudge.h>

static unsigned int quit;
static unsigned char inputbuffer[FUDGE_BSIZE];
static struct ring input;

static unsigned int complete(struct event_header *header, void *message, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);
    unsigned int id;

    if (!file_walk(FILE_CP, "/bin/complete"))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_addrequest(message, header, EVENT_INIT, id);
        event_send(message);
        event_addrequest(message, header, EVENT_DATA, id);
        event_adddata(message, 1);
        event_appenddata(message, count, command);
        event_send(message);
        event_addrequest(message, header, EVENT_EXIT, id);
        event_send(message);

    }

    return id;

}

static void printprompt(void)
{

    file_writeall(FILE_P0, "$ ", 2);

}

static void printnormal(void *buffer, unsigned int count)
{

    file_writeall(FILE_P0, "\b\b  \b\b", 6);
    file_writeall(FILE_P0, buffer, count);
    printprompt();

}

static void printcomplete(void *buffer, unsigned int count)
{

    file_writeall(FILE_P0, buffer, count);
    printprompt();

}

static unsigned int interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "cd ", 3))
    {

        command[count - 1] = '\0';

        if (file_walk(FILE_L0, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static unsigned int interpret(struct event_header *header, void *message, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);
    unsigned int id;

    if (count < 2)
        return 0;

    if (interpretbuiltin(count, command))
        return 0;

    if (!file_walk(FILE_CP, "/bin/slang"))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_addrequest(message, header, EVENT_INIT, id);
        event_send(message);
        event_addrequest(message, header, EVENT_DATA, id);
        event_adddata(message, 0);
        event_appenddata(message, count, command);
        event_send(message);
        event_addrequest(message, header, EVENT_EXIT, id);
        event_send(message);

    }

    return id;

}

static void oninit(struct event_header *header, void *message)
{

    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    file_open(FILE_P0);
    printprompt();
    file_close(FILE_P0);

}

static void onkill(struct event_header *header, void *message)
{

    event_addresponse(message, header, EVENT_EXIT);
    event_send(message);

    quit = 1;

}

static void ondata(struct event_header *header, void *message)
{

    struct event_data *data = event_getdata(header);

    file_open(FILE_P0);

    switch (data->session)
    {

    case 0:
        printnormal(data + 1, data->count);

        break;

    case 1:
        printcomplete(data + 1, data->count);

        break;

    }

    file_close(FILE_P0);


}

static void onconsoledata(struct event_header *header, void *message)
{

    struct event_consoledata *consoledata = event_getdata(header);

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(header, message, &input);

        break;

    case '\b':
    case 0x7F:
        if (!ring_skipreverse(&input, 1))
            break;

        file_open(FILE_P0);
        file_writeall(FILE_P0, "\b \b", 3);
        file_close(FILE_P0);

        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        file_open(FILE_P0);
        file_writeall(FILE_P0, &consoledata->data, 1);
        file_close(FILE_P0);
        ring_write(&input, &consoledata->data, 1);

        if (interpret(header, message, &input))
            break;

        file_open(FILE_P0);
        printprompt();
        file_close(FILE_P0);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_open(FILE_P0);
        file_writeall(FILE_P0, &consoledata->data, 1);
        file_close(FILE_P0);

        break;

    }

}

void main(void)
{

    if (!file_walk(FILE_G0, "/system/console/event"))
        return;

    file_open(FILE_G0);
    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        char message[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header, message);

            break;

        case EVENT_KILL:
            onkill(header, message);

            break;

        case EVENT_DATA:
            ondata(header, message);

            break;

        case EVENT_CONSOLEDATA:
            onconsoledata(header, message);

            break;

        }

    }

    event_close();
    file_close(FILE_G0);

}

