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

    char *request = "NICK jfu_fudge\nUSER jfu_fudge 0 * :Jens Fudge\nJOIN #fudge\n";
    char buffer[BUFFER_SIZE];
    unsigned int count;

    file_link(FILE_G0);
    socket_resolveremote(channel, FILE_G0, &local, &router);
    socket_connect_tcp(channel, FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, ascii_length(request), request);

    while ((count = socket_receive_tcp(channel, FILE_G0, &local, &remote, &router, buffer, BUFFER_SIZE)))
        channel_place(channel, EVENT_DATA, count, buffer);

    socket_disconnect_tcp(channel, FILE_G0, &local, &remote, &router);
    file_unlink(FILE_G0);
    channel_close(channel);

}

static void interpret(void *buffer, unsigned int count)
{

    char *data = buffer;

    if (data[0] == '/')
    {

        socket_send_tcp(FILE_G0, &local, &remote, &router, count - 1, data + 1);

    }

    else
    {

        char *text = "PRIVMSG #fudge :";
        char outputdata[BUFFER_SIZE];
        unsigned int offset = 0;

        offset += buffer_write(outputdata, BUFFER_SIZE, text, ascii_length(text), offset);
        offset += buffer_write(outputdata, BUFFER_SIZE, buffer, count, offset);

        socket_send_tcp(FILE_G0, &local, &remote, &router, offset, outputdata);

    }

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    char buffer[BUFFER_SIZE];
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
        channel_place(channel, EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, BUFFER_SIZE);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_place(channel, EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

void init(struct channel *channel)
{

    unsigned char address1[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port1[TCP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address2[IPV4_ADDRSIZE] = {130, 239, 18, 119};
    unsigned char port2[TCP_PORTSIZE] = {0x1A, 0x0B};
    unsigned char address3[IPV4_ADDRSIZE] = {192, 168, 0, 8};

    ring_init(&input, BUFFER_SIZE, inputbuffer);

    if (!file_walk2(FILE_L0, "system:ethernet/if:0"))
        return;

    if (!file_walk(FILE_L1, FILE_L0, "addr"))
        return;

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        return;

    socket_init(&local);
    socket_bind_ipv4(&local, address1);
    socket_bind_tcp(&local, port1, 42);
    socket_init(&remote);
    socket_bind_ipv4(&remote, address2);
    socket_bind_tcp(&remote, port2, 0);
    socket_init(&router);
    socket_bind_ipv4(&router, address3);
    socket_resolvelocal(FILE_L1, &local);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);

}

