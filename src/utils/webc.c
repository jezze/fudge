#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char name[512];

static unsigned int buildrequest(unsigned int count, void *buffer)
{

    unsigned int offset = 0;

    offset += buffer_write(buffer, count, "GET / HTTP/1.1\r\n", 16, offset);
    offset += buffer_write(buffer, count, "Host: ", 6, offset);
    offset += buffer_write(buffer, count, name, ascii_length(name), offset);
    offset += buffer_write(buffer, count, "\r\n\r\n", 4, offset);

    return offset;

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char buffer[BUFFER_SIZE];
    unsigned int count = buildrequest(BUFFER_SIZE, buffer);

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (file_walk(FILE_L0, FILE_G0, "data"))
    {

        file_link(FILE_L0);
        socket_resolveremote(channel, FILE_L0, &local, &router);
        socket_connect_tcp(channel, FILE_L0, &local, &remote, &router);
        socket_send_tcp(FILE_L0, &local, &remote, &router, count, buffer);

        while ((count = socket_receive_tcp(channel, FILE_L0, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_reply(channel, EVENT_DATA, count, buffer);

        socket_disconnect_tcp(channel, FILE_L0, &local, &remote, &router);
        file_unlink(FILE_L0);

    }

    channel_close(channel);

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "ethernet"))
    {

        file_walk2(FILE_G0, value);

    }

    if (ascii_match(key, "url"))
    {

        ascii_copy(name, value);

    }

}

void init(struct channel *channel)
{

    unsigned char address1[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port1[TCP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address2[IPV4_ADDRSIZE] = {212, 47, 234, 3};
    unsigned char port2[TCP_PORTSIZE] = {0x00, 0x50};
    unsigned char address3[IPV4_ADDRSIZE] = {10, 0, 5, 80};

    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4(&local, address1);
    socket_bind_tcp(&local, port1, 42);
    socket_init(&remote);
    socket_bind_ipv4(&remote, address2);
    socket_bind_tcp(&remote, port2, 0);
    socket_init(&router);
    socket_bind_ipv4(&router, address3);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

