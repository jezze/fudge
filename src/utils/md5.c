#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct md5 sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    md5_read(&sum, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[16];
    char output[32];
    unsigned int l = 32;
    unsigned int i;

    md5_write(&sum, digest);

    for (i = 0; i < 16; i++)
        cstring_writevalue(output, 32, digest[i], 16, 2, i * 2);

    channel_sendfmt2(CHANNEL_DEFAULT, EVENT_DATA, "%w\n", output, &l);
    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    if (!file_walk2(FILE_L0, mdata))
        channel_error("File not found");

    while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
        md5_read(&sum, buffer, count);

}

void init(void)
{

    md5_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

