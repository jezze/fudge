#include <abi.h>
#include <fudge.h>

static void dump(struct event_header *iheader, struct event_header *oheader, unsigned int count, void *buffer, unsigned int session)
{

    char *data = buffer;
    unsigned int i;

    file_write(FILE_G1, "block:\n", 7);

    for (i = 0; i < count; i++)
    {

        char num[FUDGE_NSIZE];

        file_write(FILE_G1, num, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0));

    }

    file_write(FILE_G1, "\n", 1);

}

static unsigned int ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    dump(iheader, oheader, data->count, data + 1, data->session);

    return 0;

}

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    unsigned char dummy;

    file_write(FILE_G0, &dummy, 1024);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/console/if:0/odata"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);
    event_open();

    while (!status)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_DATA:
            status = ondata(iheader, oheader);

            break;

        case EVENT_INIT:
            status = oninit(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();
    file_close(FILE_G1);
    file_close(FILE_G0);

}

