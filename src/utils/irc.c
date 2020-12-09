#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputbuffer[BUFFER_SIZE];
static struct ring input;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *request = "NICK jfu_fudge\nUSER jfu_fudge 8 x : jfu_fudge\nJOIN #fudge\n";
    unsigned char buffer[BUFFER_SIZE];
    unsigned int count;

    file_link(FILE_G0);
    socket_resolveremote(channel, FILE_G0, &local, &router);
    socket_connect_tcp(channel, FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, ascii_length(request), request);

    while ((count = socket_receive_tcp(channel, FILE_G0, &local, &remote, &router, buffer, BUFFER_SIZE)))
        channel_place(channel, source, EVENT_DATA, count, buffer);

    socket_disconnect_tcp(channel, FILE_G0, &local, &remote, &router);
    file_unlink(FILE_G0);
    channel_close(channel, source);

}

static void prep(void)
{

    char *text = "PRIVMSG #fudge :";

    ring_write(&input, text, ascii_length(text));

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    struct event_consoledata *consoledata = mdata;
    unsigned int count;

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
        ring_write(&input, &consoledata->data, 1);
        channel_place(channel, source, EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, BUFFER_SIZE);

        if (count)
            socket_send_tcp(FILE_G0, &local, &remote, &router, count, buffer);

        prep();

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_place(channel, source, EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

void init(struct channel *channel)
{

    unsigned char address1[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port1[UDP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address2[IPV4_ADDRSIZE] = {130, 239, 18, 119};
    unsigned char port2[UDP_PORTSIZE] = {0x1A, 0x0B};
    unsigned char address3[IPV4_ADDRSIZE] = {192, 168, 0, 8};

    ring_init(&input, BUFFER_SIZE, inputbuffer);
    prep();

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
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);

}

