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
    struct message message;
    unsigned int i;

    sha1_write(&sum, digest);
    message_init(&message, EVENT_DATA);

    for (i = 0; i < 20; i++)
        message_putvalue(&message, digest[i], 16, 2);

    message_putstring(&message, "\n");
    channel_sendmessage(&message);
    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_reset(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            sha1_read(&sum, buffer, count);

    }

    else
    {

        channel_error("File not found");

    }

}

void init(void)
{

    sha1_init(&sum);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

