#include <fudge.h>
#include <abi.h>

static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(CHANNEL_DEFAULT, EVENT_ERROR, msize, mdata);
    channel_close();

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = option_getdecimal("service");
    unsigned int root = fsp_walk(service, 0, "");

    if (root)
    {

        struct record records[8];
        unsigned int nrecords;
        unsigned int offset;

        for (offset = 0; (nrecords = fsp_list(service, root, offset, records)); offset += nrecords)
        {

            unsigned int i;

            for (i = 0; i < nrecords; i++)
            {

                struct record *record = &records[i];

                channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "(%u) %w\n", &record->id, record->name, &record->length);

            }

        }

    }

    if (root)
    {

        unsigned int id = fsp_walk(service, root, option_getstring("subdir"));

        if (id)
        {

            struct record records[8];
            unsigned int nrecords;
            unsigned int offset;

            for (offset = 0; (nrecords = fsp_list(service, id, offset, records)); offset += nrecords)
            {

                unsigned int i;

                for (i = 0; i < nrecords; i++)
                {

                    struct record *record = &records[i];

                    channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "(%u) %w\n", &record->id, record->name, &record->length);

                }

            }

        }

    }

    if (root)
    {

        unsigned int id = fsp_walk(service, root, option_getstring("file"));

        if (id)
        {

            char buffer[8192];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fsp_read(service, id, buffer, 8192, offset)); offset += count)
                channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

        }

    }

}

void init(void)
{

    option_add("service", "90");
    option_add("subdir", "kernel");
    option_add("file", "data/help.txt");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_ERROR, onerror);

}

