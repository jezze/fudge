#include <abi.h>
#include <fudge.h>

static struct md5 s;

static unsigned int ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    md5_read(&s, data + 1, data->count);

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
        md5_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_header *iheader, struct event_header *oheader)
{

    unsigned char digest[16];
    char num[FUDGE_NSIZE];
    unsigned int i;

    md5_write(&s, digest);
    event_replydata(oheader, iheader);

    for (i = 0; i < 16; i++)
        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0), num);

    event_appenddata(oheader, 1, "\n");
    event_send(oheader);

    return 1;

}

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    md5_init(&s);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;

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

        case EVENT_FILE:
            status = onfile(iheader, oheader);

            break;

        case EVENT_STOP:
            status = onstop(iheader, oheader);

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

}

