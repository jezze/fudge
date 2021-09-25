#include <fudge.h>
#include <abi.h>

static struct md5 sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    md5_read(&sum, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[16];
    struct message message;
    unsigned int offset = 0;
    unsigned int i;

    md5_write(&sum, digest);

    for (i = 0; i < 16; i++)
        offset = message_putvalue(&message, digest[i], 16, 2, offset);

    offset = message_putstring(&message, "\n", offset);

    channel_reply(EVENT_DATA, offset, message.data.buffer);
    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_seek(FILE_L0, 0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            md5_read(&sum, buffer, count);

    }

}

void init(void)
{

    md5_init(&sum);
    channel_setcallback(EVENT_DATA, ondata);
    channel_setcallback(EVENT_MAIN, onmain);
    channel_setcallback(EVENT_PATH, onpath);

}

