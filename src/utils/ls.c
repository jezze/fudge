#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void list(unsigned int descriptor)
{

    struct record records[8];
    unsigned int nrecords;

    call_walk_duplicate(FILE_L0, descriptor);
    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "../\n");

    while ((nrecords = call_list(descriptor, FILE_L0, 8, records)))
    {

        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            if (cstring_match(option_getstring("show"), "all"))
            {

                if (record->type == RECORD_TYPE_DIRECTORY)
                    channel_send_fmt4(CHANNEL_DEFAULT, EVENT_DATA, "%H8u %H8u %w/\n", &record->id, &record->size, record->name, &record->length);
                else
                    channel_send_fmt4(CHANNEL_DEFAULT, EVENT_DATA, "%H8u %H8u %w\n", &record->id, &record->size, record->name, &record->length);

            }

            else
            {

                if (record->type == RECORD_TYPE_DIRECTORY)
                    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "%w/\n", record->name, &record->length);
                else
                    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "%w\n", record->name, &record->length);

            }

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!paths)
        list(FILE_PW);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (call_walk_absolute(FILE_G0, mdata))
        list(FILE_G0);
    else
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

     paths++;

}

void init(void)
{

    option_add("show", "");
    call_walk_duplicate(FILE_G0, FILE_PW);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

