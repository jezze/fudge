#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void list(unsigned int source, char *path)
{

    unsigned int service = option_getdecimal("service");
    unsigned int id = fsp_walk(service, 0, path);

    if (id)
    {

        struct record records[8];
        unsigned int nrecords;
        unsigned int offset;

        channel_send_fmt0(source, EVENT_DATA, "../\n");

        for (offset = 0; (nrecords = fsp_list(service, id, offset, records)); offset += nrecords)
        {

            unsigned int i;

            for (i = 0; i < nrecords; i++)
            {

                struct record *record = &records[i];

                if (record->type == RECORD_TYPE_DIRECTORY)
                    channel_send_fmt2(source, EVENT_DATA, "%w/\n", record->name, &record->length);
                else
                    channel_send_fmt2(source, EVENT_DATA, "%w\n", record->name, &record->length);

            }

        }

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    paths++;

    list(source, mdata);

}

static void onend(unsigned int source, void *mdata, unsigned int msize)
{

    if (!paths)
        list(source, option_getstring("pwd"));

}

void init(void)
{

    option_add("service", "90");
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_END, onend);

}

