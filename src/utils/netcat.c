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
static unsigned char addr[4] = {10, 0, 5, 1};
static unsigned char port[2] = {0x07, 0xD0};

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

static void *writeethernet(void *buffer, unsigned int type, unsigned char *sha, unsigned char *tha)
{

    struct ethernet_header *header = buffer;

    ethernet_initheader(header, type, sha, tha);

    return header + 1;

}

static void *writeipv4(void *buffer, unsigned char *sip, unsigned char *tip, unsigned int protocol, unsigned int count)
{

    struct ipv4_header *header = buffer;

    ipv4_initheader(header, sip, tip, protocol, count);

    return header + 1;

}

static void *writeudp(void *buffer, unsigned char *sp, unsigned char *tp, unsigned int count)
{

    struct udp_header *header = buffer;

    udp_initheader(header, sp, tp, count);

    return header + 1;

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

/*
    file_open(FILE_G1);
    file_readall(FILE_G1, addr, IPV4_ADDRSIZE);
    file_close(FILE_G1);
*/

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

            channel_place(channel, source, EVENT_DATA, message_datasize(&header), &data);

            if (incoming.active)
            {

                struct ipv4_arpentry *sentry = findarpentry(addr);
                struct ipv4_arpentry *tentry = findarpentry(incoming.iheader.sip);
                char *buffer = data.buffer;
                unsigned int length = 1;

                if (sentry && tentry)
                {

                    buffer = writeethernet(buffer, 0x0800, sentry->haddress, tentry->haddress);
                    buffer = writeipv4(buffer, addr, incoming.iheader.sip, 0x11, sizeof (struct udp_header) + length);
                    buffer = writeudp(buffer, port, incoming.uheader.sp, length);

                    buffer_copy(buffer, &s, length);

                    buffer += length;

                    file_open(FILE_G0);
                    file_writeall(FILE_G0, data.buffer, buffer - data.buffer);
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

