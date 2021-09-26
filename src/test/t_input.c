#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring2(&message, "If you press f I will quit\n");
    channel_replymsg(&message);

    while (channel_pollsourceevent(source, EVENT_CONSOLEDATA, &message))
    {

        struct event_consoledata *consoledata = (struct event_consoledata *)message.data.buffer;
        struct message reply;

        if (consoledata->data == 'f')
        {

            message_init(&reply, EVENT_DATA);
            message_putstring2(&reply, "\nYou clicked f, goodbye!\n");
            channel_replymsg(&reply);

            break;

        }

        else if (consoledata->data == '\r')
        {

            message_init(&reply, EVENT_DATA);
            message_putstring2(&reply, "\n");
            channel_replymsg(&reply);

        }

        else
        {

            message_init(&reply, EVENT_DATA);
            message_putbuffer2(&reply, message_datasize(&message.header), message.data.buffer);
            channel_replymsg(&reply);

        }

    }

    channel_close();

}

void init(void)
{

    channel_setcallback(EVENT_MAIN, onmain);

}

