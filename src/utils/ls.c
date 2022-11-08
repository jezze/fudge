#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct record records[8];
    struct message message;
    unsigned int nrecords;
    unsigned int cid;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "../\n");
    channel_sendmessage(&message);

    for (cid = 0; (nrecords = file_list(FILE_G0, cid, 8, records)); cid = records[nrecords - 1].id)
    {

        unsigned int i;

        message_init(&message, EVENT_DATA);

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            message_putbuffer(&message, record->length, record->name);

            if (record->type == RECORD_TYPE_DIRECTORY)
                message_putstring(&message, "/");

            message_putstring(&message, "\n");

        }

        channel_sendmessage(&message);

    }

    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_L0, mdata))
        channel_error("Could not find path");

    file_duplicate(FILE_G0, FILE_L0);

}

void init(void)
{

    file_duplicate(FILE_G0, FILE_PW);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

