#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    char data[MESSAGE_SIZE];

    channel_send_fmt0(source, EVENT_DATA, "If you press f I will quit\n");

    while (channel_poll_any(EVENT_CONSOLEDATA, &message, MESSAGE_SIZE, data))
    {

        struct event_consoledata *consoledata = (void *)data;

        if (consoledata->data == 'f')
        {

            channel_send_fmt0(source, EVENT_DATA, "\nYou clicked f, goodbye!\n");

            break;

        }

        else if (consoledata->data == '\r')
        {

            channel_send_fmt0(source, EVENT_DATA, "\n");

        }

        else
        {

            channel_send_buffer(source, EVENT_DATA, message_datasize(&message), &consoledata->data);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

