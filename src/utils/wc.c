#include <abi.h>
#include <fudge.h>

static unsigned int quit;
static unsigned int bytes;
static unsigned int words;
static unsigned int lines;
static unsigned int whitespace = 1;

static void sum(struct event_header *iheader, struct event_header *oheader, unsigned int count, void *buffer, unsigned int session)
{

    char num[FUDGE_NSIZE];
    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        switch (data[i])
        {

        case '\n':
            whitespace = 1;
            lines++;

            break;

        case ' ':
            whitespace = 1;

            break;

        default:
            if (whitespace)
                words++;

            whitespace = 0;

            break;

        }

        bytes++;

    }

    event_reply(oheader, iheader, EVENT_DATA);
    event_adddata(oheader, session);
    event_appenddata(oheader, ascii_wvalue(num, FUDGE_BSIZE, lines, 10), num);
    event_appenddata(oheader, 1, "\n");
    event_appenddata(oheader, ascii_wvalue(num, FUDGE_BSIZE, words, 10), num);
    event_appenddata(oheader, 1, "\n");
    event_appenddata(oheader, ascii_wvalue(num, FUDGE_BSIZE, bytes, 10), num);
    event_appenddata(oheader, 1, "\n");
    event_send(oheader);

}

static void ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    sum(iheader, oheader, data->count, data + 1, data->session);

}

static void ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        sum(iheader, oheader, count, buffer, file->session);

    file_close(file->descriptor);

}

static void ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_DATASTOP);
    event_send(oheader);

    quit = 1;

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    quit = 1;

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_DATASTOP:
            ondatastop(iheader, oheader);

            break;

        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        case EVENT_DATA:
            ondata(iheader, oheader);

            break;

        case EVENT_DATAFILE:
            ondatafile(iheader, oheader);

            break;

        }

    }

    event_close();

}

