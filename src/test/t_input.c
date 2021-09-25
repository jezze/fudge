#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    channel_reply(EVENT_DATA, message_putstring(&message, "If you press f I will quit\n", 0), message.data.buffer);

    while (channel_pollsourceevent(source, EVENT_CONSOLEDATA, &message))
    {

        struct event_consoledata *consoledata = (struct event_consoledata *)message.data.buffer;

        if (consoledata->data == 'f')
        {

            channel_reply(EVENT_DATA, message_putstring(&message, "\nYou clicked f, goodbye!\n", 0), message.data.buffer);

            break;

        }

        else if (consoledata->data == '\r')
        {

            channel_reply(EVENT_DATA, message_putstring(&message, "\n", 0), message.data.buffer);

        }

        else
        {

            channel_reply(EVENT_DATA, message_datasize(&message.header), message.data.buffer);

        }

    }

    channel_close();

}

void init(void)
{

    channel_setcallback(EVENT_MAIN, onmain);

}

