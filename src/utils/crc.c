#include <abi.h>
#include <fudge.h>

static struct crc s;

static unsigned int ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    crc_read(&s, data + 1, data->count);

    return 0;

}

static unsigned int onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return 0;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        crc_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_header *iheader, struct event_header *oheader)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int result = crc_finalize(&s);

    event_replydata(oheader, iheader);
    event_appenddata(oheader, ascii_wvalue(buffer, 32, result, 10), buffer);
    event_appenddata(oheader, 1, "\n");
    event_send(oheader);

    return 1;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    char ibuffer[FUDGE_BSIZE];
    char obuffer[FUDGE_BSIZE];
    struct event_header *oheader = event_init(ibuffer, obuffer);

    event_open();

    while (!status)
    {

        struct event_header *iheader = event_read(ibuffer);

        switch (iheader->type)
        {

        case EVENT_DATA:
            status = ondata(iheader, oheader);

            break;

        case EVENT_FILE:
            status = onfile(iheader, oheader);

            break;

        case EVENT_STOP:
            status = onstop(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();

}

