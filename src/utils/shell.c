#include <abi.h>
#include <fudge.h>
#include <event/base.h>
#include <event/device.h>

static unsigned int quit;
static unsigned char inputbuffer[FUDGE_BSIZE];
static struct ring input;

static unsigned int interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "exit", 4))
    {

        quit = 1;

        return 1;

    }

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

static void interpretslang(unsigned int count, char *command)
{

    struct event pipe;
    unsigned int id;

    memory_copy(pipe.data, command, count);

    if (!file_walk(FILE_CP, "/bin/slang2"))
        return;

    id = call_spawn();

    event_sendinit(FILE_L1, id);
    event_send(FILE_L1, &pipe, id, EVENT_DATA, count);
    event_sendexit(FILE_L1, id);

}

static void interpret(struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    if (count < 2)
        return;

    if (!interpretbuiltin(count, command))
        interpretslang(count, command);

}

static void oninit(struct event_header *header, void *data)
{

}

static void onexit(struct event_header *header, void *data)
{

    quit = 1;

}

static void onconsoledata(struct event_header *header, void *data)
{

    struct event_consoledata *consoledata = data;

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

        file_writeall(FILE_PO, "\b \b", 3);

        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        file_writeall(FILE_PO, &consoledata->data, 1);
        ring_write(&input, &consoledata->data, 1);
        interpret(&input);
        file_writeall(FILE_PO, "$ ", 2);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_PO, &consoledata->data, 1);

        break;

    }

}

void main(void)
{

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/wm/event"))
        return;

    if (!file_walk(FILE_L2, "/system/console/event"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);
    file_open(FILE_L2);

    /* MOVE TO ONINIT */
    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    file_writeall(FILE_PO, "$ ", 2);

    while (!quit)
    {

        struct event event;

        event_read(FILE_L0, &event);

        switch (event.header.type)
        {

        case EVENT_INIT:
            oninit(&event.header, event.data);

            break;

        case EVENT_EXIT:
            onexit(&event.header, event.data);

            break;

        case EVENT_CONSOLEDATA:
            onconsoledata(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L2);
    file_close(FILE_L1);
    file_close(FILE_L0);

}

