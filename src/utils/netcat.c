#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    file_link(FILE_G0);
    socket_resolveremote(channel, FILE_G0, &local, &router);
    socket_listen_tcp(channel, FILE_G0, &local, &remote, &router);

    while ((count = socket_receive_tcp(channel, FILE_G0, &local, &remote, &router, buffer, BUFFER_SIZE)))
        channel_reply(channel, EVENT_DATA, count, buffer);

    file_unlink(FILE_G0);
    channel_close(channel);

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
        channel_reply(channel, EVENT_DATA, count, &consoledata->data);

}

void init(struct channel *channel)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port[TCP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address3[IPV4_ADDRSIZE] = {192, 168, 0, 8};

    if (!file_walk2(FILE_L0, "system:ethernet/if:0"))
        return;

    if (!file_walk(FILE_L1, FILE_L0, "addr"))
        return;

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        return;

    socket_init(&local);
    socket_bind_ipv4(&local, address);
    socket_bind_tcp(&local, port, 42);
    socket_init(&router);
    socket_bind_ipv4(&router, address3);
    socket_resolvelocal(FILE_L1, &local);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);

}

