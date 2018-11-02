#include <abi.h>
#include <fudge.h>

static void dump(struct event_header *iheader, struct event_header *oheader, unsigned int count, void *buffer, unsigned int session)
{

    char *data = buffer;
    unsigned int i;

    file_write(FILE_G4, "block:\n", 7);

    for (i = 0; i < count; i++)
    {

        char num[FUDGE_NSIZE];

        file_write(FILE_G4, num, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0));

    }

    file_write(FILE_G4, "\n", 1);

}

static unsigned int ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    dump(iheader, oheader, datapipe->count, datapipe + 1, datapipe->session);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    unsigned char dummy;

    file_walk2(FILE_G3, "/system/block/if:0/data");
    file_walk2(FILE_G4, "/system/console/if:0/odata");

    file_open(FILE_G3);
    file_open(FILE_G4);
    event_open();

    file_read(FILE_G3, &dummy, 1024);

    while (!status)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_DATAPIPE:
            status = ondatapipe(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();
    file_close(FILE_G4);
    file_close(FILE_G3);

}

