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

static void interpretslang(unsigned int count, char *command)
{

    unsigned int id = call_spawn();

    if (!id)
        return;

    event_sendinit(FILE_L0, EVENT_ADDR_SELF, id);
    event_senddata(FILE_L0, EVENT_ADDR_SELF, id, count, command);
    event_sendexit(FILE_L0, EVENT_ADDR_SELF, id);

}

static void interpret(struct ring *ring)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, buffer, FUDGE_BSIZE);

    if (interpretbuiltin(count, buffer))
        return;

    interpretslang(count, buffer);

}

static void oninit(struct event_header *header, void *data)
{

    if (!file_walk(FILE_CP, "/bin/slang"))
        return;

    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    file_open(FILE_PO);
    file_writeall(FILE_PO, "$ ", 2);
    file_close(FILE_PO);

}

static void onkill(struct event_header *header, void *data)
{

    quit = 1;

}

static void ondata(struct event_header *header, void *data)
{

    file_open(FILE_PO);
    file_writeall(FILE_PO, data, header->length - sizeof (struct event_header));
    file_close(FILE_PO);

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
        interpret(&input);
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

        struct event event;

        event_read(FILE_L0, &event);

        switch (event.header.type)
        {

        case EVENT_INIT:
            oninit(&event.header, event.data);

            break;

        case EVENT_KILL:
            onkill(&event.header, event.data);

            break;

        case EVENT_DATA:
            ondata(&event.header, event.data);

            break;

        case EVENT_CONSOLEDATA:
            onconsoledata(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L1);
    file_close(FILE_L0);

}

