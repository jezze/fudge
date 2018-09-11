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

    char buffer[FUDGE_BSIZE];

    if (!file_walk(FILE_CP, "/bin/slang"))
        return;

    if (!file_walk(FILE_LA, "/system/pipe/clone"))
        return;

    if (!file_walk(FILE_LB, "/system/pipe/clone"))
        return;

    file_open(FILE_LA);
    file_open(FILE_LB);
    file_walkfrom(FILE_CI, FILE_LA, "idata");
    file_walkfrom(FILE_LC, FILE_LA, "odata");
    file_walkfrom(FILE_LD, FILE_LB, "idata");
    file_walkfrom(FILE_CO, FILE_LB, "odata");
    call_spawn();
    file_open(FILE_LC);
    file_writeall(FILE_LC, command, count);
    file_close(FILE_LC);
    file_open(FILE_LD);

    while ((count = file_read(FILE_LD, buffer, FUDGE_BSIZE)))
        file_writeall(FILE_PO, buffer, count);

    file_close(FILE_LD);
    file_close(FILE_LB);
    file_close(FILE_LA);

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

    ring_init(&input, FUDGE_BSIZE, inputbuffer);

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/console/event"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);
    file_writeall(FILE_PO, "$ ", 2);

    while (!quit)
    {

        struct event event;

        event_read(FILE_L0, &event);

        switch (event.header.type)
        {

        case EVENT_EXIT:
            onexit(&event.header, event.data);

            break;

        case EVENT_CONSOLEDATA:
            onconsoledata(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L1);
    file_close(FILE_L0);

}

