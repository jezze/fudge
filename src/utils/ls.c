#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_reset(FILE_G0);

    do
    {

        struct record record;

        if (file_readall(FILE_G0, &record, sizeof (struct record)))
        {

            struct message message;
            unsigned int offset = 0;

            offset = message_putbuffer(&message, record.length, record.name, offset);
            offset = message_putstring(&message, "\n", offset);

            channel_reply(EVENT_DATA, offset, message.data.buffer);

        }

    } while (file_step(FILE_G0));

    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        file_duplicate(FILE_G0, FILE_L0);

}

void init(void)
{

    file_duplicate(FILE_G0, FILE_PW);
    channel_setcallback(EVENT_MAIN, onmain);
    channel_setcallback(EVENT_PATH, onpath);

}

