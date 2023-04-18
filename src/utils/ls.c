#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void list(unsigned int descriptor)
{

    struct record records[8];
    unsigned int nrecords;

    file_duplicate(FILE_L0, descriptor);
    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_DATA, "../\n");

    while ((nrecords = file_list(descriptor, FILE_L0, 8, records)))
    {

        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            if (cstring_match(option_getstring("show"), "all"))
            {

                if (record->type == RECORD_TYPE_DIRECTORY)
                    channel_sendfmt4(CHANNEL_DEFAULT, EVENT_DATA, "%H8u %H8u %w/\n", &record->id, &record->size, record->name, &record->length);
                else
                    channel_sendfmt4(CHANNEL_DEFAULT, EVENT_DATA, "%H8u %H8u %w\n", &record->id, &record->size, record->name, &record->length);

            }

            else
            {

                if (record->type == RECORD_TYPE_DIRECTORY)
                    channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "%w/\n", record->name, &record->length);
                else
                    channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "%w\n", record->name, &record->length);

            }

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!paths)
        list(FILE_PW);

    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_G0, mdata))
        list(FILE_G0);
    else
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

     paths++;

}

void init(void)
{

    option_add("show", "");
    file_duplicate(FILE_G0, FILE_PW);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

