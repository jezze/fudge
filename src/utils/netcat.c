#include <fudge.h>
#include <net.h>
#include <abi.h>

struct session
{

    unsigned int active;
    struct ipv4_header iheader;
    struct udp_header uheader;

};

static struct session incoming;
static struct ipv4_arpentry arptable[8];
static unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
static unsigned char port[UDP_PORTSIZE] = {0x07, 0xD0};

static void session_init(struct session *session, struct ipv4_header *iheader, struct udp_header *uheader)
{

    session->active = 1;

    buffer_copy(&session->iheader, iheader, sizeof (struct ipv4_header));
    buffer_copy(&session->uheader, uheader, sizeof (struct udp_header));

}

static struct ipv4_arpentry *findarpentry(void *paddress)
{

    unsigned int i;

    file_open(FILE_G2);
    file_readall(FILE_G2, arptable, sizeof (struct ipv4_arpentry) * 8);
    file_close(FILE_G2);

    for (i = 0; i < 8; i++)
    {

        if (buffer_match(arptable[i].paddress, paddress, IPV4_ADDRSIZE))
            return &arptable[i];

    }

    return 0;

}

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

                        if (!incoming.active)
                            session_init(&incoming, iheader, uheader);

                        channel_place(channel, source, EVENT_DATA, length, payload);

                    }

                }

            }

        }

        if (header.event == EVENT_CONSOLEDATA)
        {

            struct event_consoledata *consoledata = (struct event_consoledata *)data.buffer;
            char s = consoledata->data;
            unsigned int length = 1;

            channel_place(channel, source, EVENT_DATA, length, &s);

            if (incoming.active)
            {

                struct ipv4_arpentry *sentry = findarpentry(address);
                struct ipv4_arpentry *tentry = findarpentry(incoming.iheader.sip);

                if (sentry && tentry)
                {

                    struct ethernet_header eheader;
                    struct ipv4_header iheader;
                    struct udp_header uheader;
                    unsigned int offset = 0;

                    ethernet_initheader(&eheader, 0x0800, sentry->haddress, tentry->haddress);
                    ipv4_initheader(&iheader, address, incoming.iheader.sip, 0x11, sizeof (struct udp_header) + length);
                    udp_initheader(&uheader, port, incoming.uheader.sp, length);

                    offset = message_putbuffer(&data, sizeof (struct ethernet_header), &eheader, offset);
                    offset = message_putbuffer(&data, sizeof (struct ipv4_header), &iheader, offset);
                    offset = message_putbuffer(&data, sizeof (struct udp_header), &uheader, offset);
                    offset = message_putbuffer(&data, length, &s, offset);

                    file_open(FILE_G0);
                    file_writeall(FILE_G0, data.buffer, offset);
                    file_close(FILE_G0);

                }

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

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    if (!file_walk2(FILE_G2, "/system/ethernet/ipv4/arptable"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

