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

            message_init(&message, EVENT_DATA);
            message_putvalue(&message, record.id, 16, 8);
            message_putstring(&message, " ");
            message_putvalue(&message, record.size, 16, 8);
            message_putstring(&message, " ");
            message_putbuffer(&message, record.length, record.name);
            message_putstring(&message, "\n");
            channel_sendmessage(&message);

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
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

