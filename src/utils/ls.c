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

            unsigned char data[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset;

            channel_send_fmt0(0, source, EVENT_DATA, "../\n");

            /* Should use MESSAGE_SIZE here */
            for (offset = 0; (count = fs_read(1, target, id, data, sizeof (struct record) * 8, offset)); offset += count)
            {

                unsigned int i;

                for (i = 0; i < count; i += sizeof (struct record))
                {

                    struct record *record = (struct record *)(data + i);

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

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    if (!paths)
        list(source, option_getstring("pwd"));

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

    while (channel_process(0));

}

