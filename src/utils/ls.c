#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void list(char *path)
{

    unsigned int service = option_getdecimal("service");
    unsigned int root = fsp_walk(service, 0, "");

    if (root)
    {

        unsigned int id = fsp_walk(service, root, path);

        if (id)
        {

            struct record records[8];
            unsigned int nrecords;
            unsigned int offset;

            channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "../\n");

            for (offset = 0; (nrecords = fsp_list(service, id, offset, records)); offset += nrecords)
            {

                unsigned int i;

                for (i = 0; i < nrecords; i++)
                {

                    struct record *record = &records[i];

                    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "%w\n", record->name, &record->length);

                }

            }

        }

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    paths++;

    list(mdata);

}

static void onend(unsigned int source, void *mdata, unsigned int msize)
{

    if (!paths)
        list(option_getstring("pwd"));

}

void init(void)
{

    option_add("service", "90");
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_END, onend);

}

