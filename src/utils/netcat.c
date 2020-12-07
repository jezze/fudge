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

    socket_listen(FILE_G0, IPV4_PROTOCOL_TCP, &local);

    while (channel_polldescriptor(channel, FILE_G0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            unsigned char buffer[BUFFER_SIZE];
            unsigned int count;

            socket_handle_arp(FILE_G0, &local, &remote, message_datasize(&header), &data);

            count = socket_handle_tcp(FILE_G0, &local, &remote, &router, message_datasize(&header), &data, BUFFER_SIZE, buffer);

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
        count = socket_send_tcp(FILE_G0, &local, &remote, &router, 1, &consoledata->data);

        break;

    default:
        count = socket_send_tcp(FILE_G0, &local, &remote, &router, 1, &consoledata->data);

        break;

    }

    if (count)
        channel_place(channel, source, EVENT_DATA, count, &consoledata->data);

}

void init(struct channel *channel)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port[UDP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address3[IPV4_ADDRSIZE] = {192, 168, 0, 8};

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    socket_init(&local);
    socket_bind(&local, address);
    socket_bind_tcp(&local, port, 42);
    socket_init(&router);
    socket_bind(&router, address3);
    socket_resolvelocal(FILE_G1, &local);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);

}

