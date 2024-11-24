#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void list(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, path);

        if (id)
        {

            struct record records[8];
            unsigned int nrecords;
            unsigned int offset;

            channel_send_fmt0(0, source, EVENT_DATA, "../\n");

            for (offset = 0; (nrecords = fs_list(1, target, id, offset, records, 8)); offset += nrecords)
            {

                unsigned int i;

                for (i = 0; i < nrecords; i++)
                {

                    struct record *record = &records[i];

                    if (record->type == RECORD_TYPE_DIRECTORY)
                        channel_send_fmt2(0, source, EVENT_DATA, "%w/\n", record->name, &record->length);
                    else
                        channel_send_fmt2(0, source, EVENT_DATA, "%w\n", record->name, &record->length);

                }

            }

        }

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    paths++;

    list(source, mdata);

}

static void onexit(unsigned int source, void *mdata, unsigned int msize)
{

    if (!paths)
        list(source, option_getstring("pwd"));

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_EXIT, onexit);

}

