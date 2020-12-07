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
    socket_resolveremote(FILE_G0, &local, &remote);

    while (channel_polldescriptor(channel, FILE_G0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            socket_handle_arp(FILE_G0, &local, &remote, message_datasize(&header), &data);

            if (remote.resolved)
                break;

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

