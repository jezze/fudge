#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct record records[8];
    unsigned int nrecords;

    file_duplicate(FILE_L0, FILE_G0);
    channel_sendstring(EVENT_DATA, "../\n");

    while ((nrecords = file_list(FILE_G0, FILE_L0, 8, records)))
    {

        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            if (record->type == RECORD_TYPE_DIRECTORY)
                channel_sendfmt4(EVENT_DATA, "%H8u %H8u %w/\n", &record->id, &record->size, record->name, &record->length);
            else
                channel_sendfmt4(EVENT_DATA, "%H8u %H8u %w\n", &record->id, &record->size, record->name, &record->length);

        }

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

