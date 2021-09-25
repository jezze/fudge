#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    channel_reply(EVENT_DATA, message_putstring(&data, "If you press f I will quit\n", 0), &data);

    while (channel_pollsourceevent(source, EVENT_CONSOLEDATA, &header, &data))
    {

        struct event_consoledata *consoledata = (struct event_consoledata *)data.buffer;

        if (consoledata->data == 'f')
        {

            channel_reply(EVENT_DATA, message_putstring(&data, "\nYou clicked f, goodbye!\n", 0), &data);

            break;

        }

        else if (consoledata->data == '\r')
        {

            channel_reply(EVENT_DATA, message_putstring(&data, "\n", 0), &data);

        }

        else
        {

            channel_reply(EVENT_DATA, message_datasize(&header), &data);

        }

    }

    channel_close();

}

void init(void)
{

    channel_setcallback(EVENT_MAIN, onmain);

}

