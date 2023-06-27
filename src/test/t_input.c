#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    union {struct event_consoledata consoledata; char data[MESSAGE_SIZE];} data;

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "If you press f I will quit\n");

    while (channel_poll_from(source, EVENT_CONSOLEDATA, &message, &data))
    {

        if (data.consoledata.data == 'f')
        {

            channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "\nYou clicked f, goodbye!\n");

            break;

        }

        else if (data.consoledata.data == '\r')
        {

            channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "\n");

        }

        else
        {

            channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, message_datasize(&message), &data.consoledata.data);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

