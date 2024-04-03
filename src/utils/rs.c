#include <fudge.h>
#include <abi.h>

static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(CHANNEL_DEFAULT, EVENT_ERROR, msize, mdata);
    channel_close();

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = option_getdecimal("fs-service");
    unsigned int root = fsp_walk(target, 0, "");

    if (root)
    {

        struct record records[8];
        unsigned int nrecords = fsp_list(target, root, records);
        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "(%u) %w\n", &record->id, record->name, &record->length);

        }

    }

    /*
    if (root)
    {

        unsigned int id = fsp_walk(target, root, "test.txt");

        if (id)
        {

            char buffer[8192];
            unsigned int count = fsp_read(target, id, 100, 0, 8192, buffer);

            if (count)
                channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

        }

    }
    */

}

void init(void)
{

    option_add("fs-service", "1111");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_ERROR, onerror);

}

