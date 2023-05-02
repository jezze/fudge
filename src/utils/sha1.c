#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct sha1 sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    sha1_read(&sum, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[20];
    char output[40];
    unsigned int l = 40;
    unsigned int i;

    sha1_write(&sum, digest);

    for (i = 0; i < 20; i++)
        cstring_writevalue(output, 40, digest[i], 16, 2, i * 2);

    channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "%w\n", output, &l);
    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;
        unsigned int offset;

        for (offset = 0; (count = file_read(FILE_L0, buffer, BUFFER_SIZE, offset)); offset += count)
            sha1_read(&sum, buffer, count);

    }

    else
    {

        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    sha1_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

