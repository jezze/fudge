#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct crc sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    crc_read(&sum, mdata, msize);

}

static void onexit(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int crc = crc_finalize(&sum);

    channel_send_fmt1(source, EVENT_DATA, "%u\n", &crc);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fsp_auth(mdata);

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, mdata);

        if (id)
        {

            char buffer[4096];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fsp_read(service, id, buffer, 4096, offset)); offset += count)
                crc_read(&sum, buffer, count);

        }

        else
        {

            channel_send_fmt1(source, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_PATH, onpath);

}

