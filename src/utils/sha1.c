#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct sha1 sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    sha1_read(&sum, mdata, msize);

}

static void onexit(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[20];
    char output[40];
    unsigned int l = 40;
    unsigned int i;

    sha1_write(&sum, digest);

    for (i = 0; i < 20; i++)
        cstring_write_value(output, 40, digest[i], 16, 2, i * 2);

    channel_send_fmt2(0, source, EVENT_DATA, "%w\n", output, &l);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fs_auth(mdata);

    if (service)
    {

        unsigned int id = fs_walk(0, service, 0, mdata);

        if (id)
        {
 
            char buffer[4096];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(0, service, id, buffer, 4096, offset)); offset += count)
                sha1_read(&sum, buffer, count);

        }

        else
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

}

void init(void)
{

    sha1_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_PATH, onpath);

}

