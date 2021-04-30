#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char request[1024];
static unsigned int requestlength;

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "url"))
    {

        requestlength += buffer_write(request, 1024, "GET / HTTP/1.1\r\n", 16, requestlength);
        requestlength += buffer_write(request, 1024, "Host: ", 6, requestlength);
        requestlength += buffer_write(request, 1024, value, ascii_length(value), requestlength);
        requestlength += buffer_write(request, 1024, "\r\n\r\n", 4, requestlength);

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (requestlength > 0)
    {

        unsigned char buffer[BUFFER_SIZE];
        unsigned int count;

        file_link(FILE_G0);
        socket_resolveremote(channel, FILE_G0, &local, &router);
        socket_connect_tcp(channel, FILE_G0, &local, &remote, &router);
        socket_send_tcp(FILE_G0, &local, &remote, &router, requestlength, request);

        while ((count = socket_receive_tcp(channel, FILE_G0, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_reply(channel, EVENT_DATA, count, buffer);

        socket_disconnect_tcp(channel, FILE_G0, &local, &remote, &router);
        file_unlink(FILE_G0);

    }

    channel_close(channel);

}

void init(struct channel *channel)
{

    unsigned char address1[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port1[TCP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address2[IPV4_ADDRSIZE] = {212, 47, 234, 3};
    unsigned char port2[TCP_PORTSIZE] = {0x00, 0x50};
    unsigned char address3[IPV4_ADDRSIZE] = {10, 0, 5, 80};

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
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

