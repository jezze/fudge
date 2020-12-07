#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);
    socket_resolveremote(FILE_G0, &local, &router);

    while (channel_polldescriptor(channel, FILE_G0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            socket_handle_arp(FILE_G0, &local, &router, message_datasize(&header), &data);

            if (router.resolved)
                break;

        }

    }

    socket_connect(FILE_G0, IPV4_PROTOCOL_TCP, &local, &remote, &router);

    while (channel_polldescriptor(channel, FILE_G0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            unsigned char buffer[BUFFER_SIZE];

            socket_handle_tcp(FILE_G0, &local, &remote, &router, message_datasize(&header), &data, BUFFER_SIZE, buffer);

            if (local.info.tcp.state == TCP_STATE_ESTABLISHED)
            {

                char *request = "GET / HTTP/1.1\r\nHost: www.blunder.se\r\n\r\n";

                socket_send_tcp(FILE_G0, &local, &remote, &router, ascii_length(request), request);

                break;

            }

        }

    }

    while (channel_polldescriptor(channel, FILE_G0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            unsigned char buffer[BUFFER_SIZE];
            unsigned int count = socket_handle_tcp(FILE_G0, &local, &remote, &router, message_datasize(&header), &data, BUFFER_SIZE, buffer);

            if (count)
                channel_place(channel, source, EVENT_DATA, count, buffer);

            if (local.info.tcp.state == TCP_STATE_CLOSED)
                break;

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    unsigned char address1[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port1[UDP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address2[IPV4_ADDRSIZE] = {212, 47, 234, 3};
    unsigned char port2[UDP_PORTSIZE] = {0x00, 0x50};
    unsigned char address3[IPV4_ADDRSIZE] = {192, 168, 0, 8};

    if (!file_walk2(FILE_L0, "/system/ethernet/if:0"))
        return;

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        return;

    if (!file_walk(FILE_G1, FILE_L0, "addr"))
        return;

    socket_init(&local);
    socket_bind(&local, address1);
    socket_bind_tcp(&local, port1, 42);
    socket_init(&remote);
    socket_bind(&remote, address2);
    socket_bind_tcp(&remote, port2, 0);
    socket_init(&router);
    socket_bind(&router, address3);
    socket_resolvelocal(FILE_G1, &local);
    channel_setcallback(channel, EVENT_MAIN, onmain);

}

