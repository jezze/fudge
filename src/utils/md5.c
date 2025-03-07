#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct md5 sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    md5_read(&sum, mdata, msize);

}

static void onexit(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[16];
    char output[32];
    unsigned int l = 32;
    unsigned int i;

    md5_write(&sum, digest);

    for (i = 0; i < 16; i++)
        cstring_write_value(output, 32, digest[i], 16, 2, i * 2);

    channel_send_fmt2(0, source, EVENT_DATA, "%w\n", output, &l);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_auth(mdata);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, mdata);

        if (id)
        {
 
            char buffer[4096];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, 4096, offset)); offset += count)
                md5_read(&sum, buffer, count);

        }

        else
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

}

void init(void)
{

    md5_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_EXIT, onexit);
    channel_bind(EVENT_PATH, onpath);

}

