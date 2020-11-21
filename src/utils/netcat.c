#include <fudge.h>
#include <net.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);

    while (channel_poll(channel, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            struct ethernet_header *eheader = (struct ethernet_header *)data.buffer;

            if (eheader->type[0] == 0x08 && eheader->type[1] == 0x00)
            {

                struct ipv4_header *iheader = (struct ipv4_header *)(eheader + 1);

                if (iheader->protocol == 0x11)
                {

                    struct udp_header *uheader = (struct udp_header *)(iheader + 1);

                    if (uheader->tp[0] == 0x07 && uheader->tp[1] == 0xD0)
                    {

                        char *payload = (char *)(uheader + 1);
                        unsigned int length = (uheader->length[1] | uheader->length[0] << 8) - sizeof (struct udp_header);

                        channel_place(channel, source, EVENT_DATA, length, payload);

                    }

                }

            }

        }

        if (header.event == EVENT_CONSOLEDATA)
        {

            struct event_consoledata *consoledata = (struct event_consoledata *)data.buffer;

            if (consoledata->data == 'f')
            {

                channel_place(channel, source, EVENT_DATA, message_putstring(&data, "\nYou clicked f, goodbye!\n", 0), &data);

                break;

            }

            else if (consoledata->data == '\r')
            {
 
                channel_place(channel, source, EVENT_DATA, message_putstring(&data, "\n", 0), &data);

            }

            else
            {

                channel_place(channel, source, EVENT_DATA, message_datasize(&header), &data);

            }

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

