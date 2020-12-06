#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);
    socket_listen(FILE_G0, IPV4_PROTOCOL_TCP, &local);

    while (channel_polldescriptor(channel, FILE_G0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            unsigned char buffer[BUFFER_SIZE];
            unsigned int count;

            count = socket_arp_read(message_datasize(&header), &data, BUFFER_SIZE, &buffer);

            if (count)
            {

                struct arp_header *aheader = (struct arp_header *)buffer;

                switch (net_load16(aheader->operation))
                {

                case ARP_REQUEST:
                    if (buffer_match(buffer + arp_hlen(aheader) + aheader->hlength + aheader->plength + aheader->hlength, local.paddress, IPV4_ADDRSIZE))
                    {

                        struct socket answer;

                        socket_init(&answer);
                        buffer_copy(answer.haddress, buffer + arp_hlen(aheader), ETHERNET_ADDRSIZE);
                        buffer_copy(answer.paddress, buffer + arp_hlen(aheader) + aheader->hlength, IPV4_ADDRSIZE);

                        file_open(FILE_G0);
                        file_writeall(FILE_G0, &data, socket_arp_build(&local, &answer, &answer, &data, ARP_REPLY, local.haddress, local.paddress, buffer + arp_hlen(aheader), buffer + arp_hlen(aheader) + aheader->hlength));
                        file_close(FILE_G0);

                    }

                    break;

                }

            }

            /* Add router lookup */
            count = socket_tcp_handle(FILE_G0, &local, &remote, &remote, message_datasize(&header), &data, BUFFER_SIZE, &buffer);

            if (count)
                channel_place(channel, source, EVENT_DATA, count, buffer);

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    unsigned int count = 0;

    if (!remote.resolved)
        return;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        break;

    case '\b':
    case 0x7F:
        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        count = socket_tcp_send(FILE_G0, &local, &remote, &remote, 1, &consoledata->data);

        break;

    default:
        count = socket_tcp_send(FILE_G0, &local, &remote, &remote, 1, &consoledata->data);

        break;

    }

    if (count)
        channel_place(channel, source, EVENT_DATA, count, &consoledata->data);

}

void init(struct channel *channel)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port[UDP_PORTSIZE] = {0x07, 0xD0};

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    socket_init(&local);
    socket_bind(&local, address);
    socket_bind_tcp(&local, port, 42);
    socket_resolvelocal(FILE_G1, &local);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);

}

