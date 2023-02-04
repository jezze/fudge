#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    struct event_consoledata consoledata;

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_DATA, "If you press f I will quit\n");

    while (channel_pollfrom(source, EVENT_CONSOLEDATA, &message, sizeof (struct event_consoledata), &consoledata))
    {

        if (consoledata.data == 'f')
        {

            channel_sendfmt0(CHANNEL_DEFAULT, EVENT_DATA, "\nYou clicked f, goodbye!\n");

            break;

        }

        else if (consoledata.data == '\r')
        {

            channel_sendfmt0(CHANNEL_DEFAULT, EVENT_DATA, "\n");

        }

        else
        {

            channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, message_datasize(&message), &consoledata.data);

        }

    }

    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

