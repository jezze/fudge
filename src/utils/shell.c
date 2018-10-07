#include <abi.h>
#include <fudge.h>

static unsigned int quit;
static unsigned char inputbuffer[FUDGE_BSIZE];
static struct ring input;

static unsigned int complete(struct event_header *iheader, struct event_header *oheader, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);
    unsigned int id;

    if (!file_walk(FILE_CP, "/bin/complete"))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_request(oheader, iheader, EVENT_INIT, id);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_DATAPIPE, id);
        event_adddatapipe(oheader, 1);
        event_appenddata(oheader, count, command);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_DATASTOP, id);
        event_adddatastop(oheader, 1);
        event_send(oheader);

    }

    return id;

}

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

static unsigned int interpret(struct event_header *iheader, struct event_header *oheader, struct ring *ring)
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

        event_request(oheader, iheader, EVENT_INIT, id);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_DATAPIPE, id);
        event_adddatapipe(oheader, 0);
        event_appenddata(oheader, count, command);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_DATASTOP, id);
        event_adddatastop(oheader, 0);
        event_send(oheader);

    }

    return id;

}

static void oninit(struct event_header *iheader, struct event_header *oheader)
{

    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    printprompt();

}

static void ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datastop *datastop = event_getdata(iheader);

    event_reply(oheader, iheader, EVENT_DATASTOP);
    event_adddatastop(oheader, datastop->session);
    event_send(oheader);

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    quit = 1;

}

static void ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    switch (datapipe->session)
    {

    case 0:
        printnormal(datapipe + 1, datapipe->count);

        break;

    case 1:
        printcomplete(datapipe + 1, datapipe->count);

        break;

    }

}

static void onconsoledata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_consoledata *consoledata = event_getdata(iheader);

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(iheader, oheader, &input);

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

        if (interpret(iheader, oheader, &input))
            break;

        printprompt();

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

}

void main(void)
{

    if (!file_walkfrom(FILE_G0, FILE_P0, "event"))
        return;

    if (!file_walkfrom(FILE_G1, FILE_P0, "odata"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);
    event_open();

    while (!quit)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_INIT:
            oninit(iheader, oheader);

            break;

        case EVENT_DATASTOP:
            ondatastop(iheader, oheader);

            break;

        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        case EVENT_DATAPIPE:
            ondatapipe(iheader, oheader);

            break;

        case EVENT_CONSOLEDATA:
            onconsoledata(iheader, oheader);

            break;

        }

    }

    event_close();
    file_close(FILE_G1);
    file_close(FILE_G0);

}

