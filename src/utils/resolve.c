#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;
    struct socket remote;

    file_link(FILE_G0);
    socket_init(&remote);
    socket_bind(&remote, mdata);
    socket_resolveremote(FILE_G0, &local, &remote, &remote);

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

                case ARP_REPLY:
                    channel_place(channel, source, EVENT_DATA, aheader->hlength, buffer + arp_hlen(aheader));

                    break;

                }

            }

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    socket_init(&local);
    socket_bind(&local, address);
    socket_resolvelocal(FILE_G1, &local);
    channel_setcallback(channel, EVENT_DATA, ondata);

}

