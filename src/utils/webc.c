#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char url[512];

static unsigned int buildrequest(unsigned int count, void *buffer)
{

    unsigned int offset = 0;

    offset += buffer_write(buffer, count, "GET / HTTP/1.1\r\n", 16, offset);
    offset += buffer_write(buffer, count, "Host: ", 6, offset);
    offset += buffer_write(buffer, count, url, ascii_length(url), offset);
    offset += buffer_write(buffer, count, "\r\n\r\n", 4, offset);

    return offset;

}

static void resolve(struct channel *channel, char *domain)
{

    unsigned int id = file_spawn("/bin/dns");

    if (id)
    {

        struct message_header header;
        struct message_data data;

        channel_sendredirectback(channel, id, EVENT_DATA);
        channel_sendredirectback(channel, id, EVENT_CLOSE);
        channel_send(channel, id, EVENT_OPTION, message_putstringz(&data, domain, message_putstringz(&data, "domain", 0)), &data);
        channel_send(channel, id, EVENT_QUERY, message_putstringz(&data, "data", 0), &data);

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.event == EVENT_CLOSE)
                break;

            if (header.event == EVENT_DATA)
            {

                socket_bind_ipv4s(&remote, data.buffer);
                socket_bind_tcps(&remote, "80", 0);
                channel_reply(channel, EVENT_DATA, ascii_length(data.buffer), data.buffer);

            }

        }

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    /* need to cut away only domain here */
    resolve(channel, url);

    if (file_walk(FILE_L0, FILE_G0, "data"))
    {

        unsigned char buffer[BUFFER_SIZE];
        unsigned int count = buildrequest(BUFFER_SIZE, buffer);

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
        file_walk2(FILE_G0, value);

    if (ascii_match(key, "url"))
        ascii_copy(url, value);

    if (ascii_match(key, "local-address"))
        socket_bind_ipv4s(&local, value);

    if (ascii_match(key, "local-port"))
        socket_bind_tcps(&local, value, 42);

    if (ascii_match(key, "remote-address") || ascii_match(key, "address"))
        socket_bind_ipv4s(&remote, value);

    if (ascii_match(key, "remote-port") || ascii_match(key, "port"))
        socket_bind_tcps(&remote, value, 42);

    if (ascii_match(key, "router-address"))
        socket_bind_ipv4s(&router, value);

}

void init(struct channel *channel)
{

    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_tcps(&local, "50001", 42);
    socket_init(&remote);
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

