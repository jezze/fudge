#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct crc sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    crc_read(&sum, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int crc = crc_finalize(&sum);

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "%u\n", &crc);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (call_walk_absolute(FILE_L0, mdata))
    {

        char buffer[4096];
        unsigned int count;
        unsigned int offset;

        for (offset = 0; (count = call_read(FILE_L0, buffer, 4096, offset)); offset += count)
            crc_read(&sum, buffer, count);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

