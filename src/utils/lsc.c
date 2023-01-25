#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct record records[8];
    struct message message;
    unsigned int nrecords;

    file_duplicate(FILE_L0, FILE_G0);
    message_init(&message, EVENT_DATA);
    message_putstring(&message, "../\n");
    channel_sendmessage(&message);

    while ((nrecords = file_list(FILE_G0, FILE_L0, 8, records)))
    {

        unsigned int i;

        message_init(&message, EVENT_DATA);

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            if (record->type == RECORD_TYPE_DIRECTORY)
                message_putfmt4(&message, "%H8u %H8u %w/\n", &record->id, &record->size, record->name, &record->length);
            else
                message_putfmt4(&message, "%H8u %H8u %w\n", &record->id, &record->size, record->name, &record->length);

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

