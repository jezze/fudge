#include <fudge.h>
#include <abi.h>

static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(CHANNEL_DEFAULT, EVENT_ERROR, msize, mdata);
    channel_close();

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int channel = option_getdecimal("service");
    unsigned int root = fsp_walk(channel, 0, "");

    if (root)
    {

        struct record records[8];
        unsigned int nrecords = fsp_list(channel, root, 0, records);
        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "(%u) %w\n", &record->id, record->name, &record->length);

        }

    }

    if (root)
    {

        unsigned int id = fsp_walk(channel, root, "data/help.txt");

        if (id)
        {

            char buffer[8192];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fsp_read(channel, id, MESSAGE_SIZE, offset, 8192, buffer)); offset += count)
                channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

        }

    }

}

void init(void)
{

    option_add("service", "666");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_ERROR, onerror);

}

