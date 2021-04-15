#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    channel_reply(channel, EVENT_DATA, message_putstring(&data, "If you press f I will quit\n", 0), &data);

    while (channel_pollsource(channel, source, &header, &data))
    {

        if (header.event == EVENT_CONSOLEDATA)
        {

            struct event_consoledata *consoledata = (struct event_consoledata *)data.buffer;

            if (consoledata->data == 'f')
            {

                channel_reply(channel, EVENT_DATA, message_putstring(&data, "\nYou clicked f, goodbye!\n", 0), &data);

                break;

            }

            else if (consoledata->data == '\r')
            {
 
                channel_reply(channel, EVENT_DATA, message_putstring(&data, "\n", 0), &data);

            }

            else
            {

                channel_reply(channel, EVENT_DATA, message_datasize(&header), &data);

            }

        }

    }

    channel_close(channel);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

