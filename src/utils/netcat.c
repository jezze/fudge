#include <fudge.h>
#include <net.h>
#include <abi.h>

struct session
{

    unsigned int active;
    struct ethernet_header eheader;
    struct ipv4_header iheader;
    struct udp_header uheader;

};

static struct session ins[32];
static struct ipv4_arpentry arptable[8];

static unsigned int savesession(struct ethernet_header *eheader, struct ipv4_header *iheader, struct udp_header *uheader)
{

    unsigned int index = 1;
    struct session *in = &ins[index];

    in->active = 1;

    buffer_copy(&in->eheader, eheader, sizeof (struct ethernet_header));
    buffer_copy(&in->iheader, iheader, sizeof (struct ipv4_header));
    buffer_copy(&in->uheader, uheader, sizeof (struct udp_header));

    return index;

}

static void *writeethernet(void *buffer, unsigned int type, unsigned char *sha, unsigned char *tha)
{

    struct ethernet_header *header = buffer;

    ethernet_initheader(header, type, sha, tha);

    return header + 1;

}

static struct ipv4_arpentry *findarpentry(void *paddress)
{

    unsigned int i;

    file_open(FILE_G1);
    file_readall(FILE_G1, arptable, sizeof (struct ipv4_arpentry) * 8);
    file_close(FILE_G1);

    for (i = 0; i < 8; i++)
    {

        if (buffer_match(arptable[i].paddress, paddress, IPV4_ADDRSIZE))
            return &arptable[i];

    }

    return 0;

}

static void *writeipv4(void *buffer, unsigned char *sip, unsigned char *tip, unsigned int protocol, unsigned int count)
{

    struct ipv4_arpentry *sentry = findarpentry(sip);
    struct ipv4_arpentry *tentry = findarpentry(tip);
    struct ipv4_header *header;

    if (!sentry || !tentry)
        return 0;

    header = writeethernet(buffer, 0x0800, sentry->haddress, tentry->haddress);

    if (!header)
        return 0;

    ipv4_initheader(header, sip, tip, protocol, count);

    return header + 1;

}

static void *writeudp(void *buffer, unsigned char *sip, unsigned char *sp, unsigned char *tip, unsigned char *tp, unsigned int count)
{

    struct udp_header *header = writeipv4(buffer, sip, tip, 0x11, count + 8);

    if (!header)
        return 0;

    udp_initheader(header, sp, tp, count);

    return header + 1;

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

                        savesession(eheader, iheader, uheader);
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

            if (ins[1].active)
            {

                char c = consoledata->data;
                unsigned char ip[4] = {10, 0, 5, 1};
                unsigned char port[2] = {0x07, 0xD0};
                void *payload = writeudp(data.buffer, ip, port, ins[1].iheader.sip, ins[1].uheader.sp, 1);

                if (payload)
                    buffer_copy(payload, &c, 1);

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

    if (!file_walk2(FILE_G1, "/system/ethernet/ipv4/arptable"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

