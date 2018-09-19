#include <abi.h>
#include <fudge.h>
#include <event/base.h>
#include <event/device.h>

static unsigned int quit;
static unsigned char inputbuffer[FUDGE_BSIZE];
static struct ring input;

static unsigned int interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "cd ", 3))
    {

        command[count - 1] = '\0';

        if (file_walk(FILE_L8, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L8);
            file_duplicate(FILE_CW, FILE_L8);

        }

        return 1;

    }

    return 0;

}

static unsigned int interpret(struct event_header *header, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);
    unsigned int id;

    if (count < 2)
        return 0;

    if (interpretbuiltin(count, command))
        return 0;

    id = call_spawn();

    if (id)
    {

        char buffer[FUDGE_BSIZE];

        event_addrequest(buffer, header, EVENT_INIT, id);
        event_sendbuffer(FILE_L0, buffer);
        event_addrequest(buffer, header, EVENT_DATA, id);
        event_adddata(buffer, count, command);
        event_sendbuffer(FILE_L0, buffer);
        event_addrequest(buffer, header, EVENT_EXIT, id);
        event_sendbuffer(FILE_L0, buffer);

    }

    return id;

}

static void oninit(struct event_header *header)
{

    if (!file_walk(FILE_CP, "/bin/slang"))
        return;

    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    file_open(FILE_PO);
    file_writeall(FILE_PO, "$ ", 2);
    file_close(FILE_PO);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addreply(message, header, EVENT_CHILD);
    event_sendbuffer(FILE_L0, message);

    quit = 1;

}

static void ondata(struct event_header *header)
{

    struct event_data *data = event_getdata(header);

    file_open(FILE_PO);
    file_writeall(FILE_PO, data + 1, data->count);
    file_close(FILE_PO);

}

static void onchild(struct event_header *header)
{

    file_open(FILE_PO);
    file_writeall(FILE_PO, "$ ", 2);
    file_close(FILE_PO);

}

static void onconsoledata(struct event_header *header)
{

    struct event_consoledata *consoledata = event_getdata(header);

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        /* Call complete */

        break;

    case '\b':
    case 0x7F:
        if (!ring_skipreverse(&input, 1))
            break;

        file_open(FILE_PO);
        file_writeall(FILE_PO, "\b \b", 3);
        file_close(FILE_PO);

        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        file_open(FILE_PO);
        file_writeall(FILE_PO, &consoledata->data, 1);
        file_close(FILE_PO);
        ring_write(&input, &consoledata->data, 1);

        if (interpret(header, &input))
            break;

        file_open(FILE_PO);
        file_writeall(FILE_PO, "$ ", 2);
        file_close(FILE_PO);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_open(FILE_PO);
        file_writeall(FILE_PO, &consoledata->data, 1);
        file_close(FILE_PO);

        break;

    }

}

void main(void)
{

    /* Once event system is inside kernel, there is no need to use the filesystem to send events */
    file_walk(FILE_L0, "/system/event");
    file_open(FILE_L0);

    if (!file_walk(FILE_L1, "/system/console/event"))
        return;

    file_open(FILE_L1);

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(FILE_L0, data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header);

            break;

        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_DATA:
            ondata(header);

            break;

        case EVENT_CHILD:
            onchild(header);

            break;

        case EVENT_CONSOLEDATA:
            onconsoledata(header);

            break;

        }

    }

    file_close(FILE_L1);
    file_close(FILE_L0);

}

