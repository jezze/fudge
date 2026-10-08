#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket router;
static struct socket local;
static struct socket remotes[64];
static char inputdata[4096];
static struct ring input;
static char request[128];

static void sendresponse(unsigned int ethernet, unsigned int source, struct socket *remote)
{

    unsigned int target = fs_auth("initrd:");
    unsigned int root = fs_walk(1, target, 0, "data/html");
    unsigned int id;
    struct record record;
    char buffer[4096];
    unsigned int count = 0;

    if (cstring_length(request) == 1 && request[0] == '/')
        cstring_write_zero(request, 128, cstring_write(request, 128, "/index.html", 0));

    id = fs_walk(1, target, root, request + 1);

    if (id && fs_stat(1, target, id, &record))
    {

        unsigned int offset;

        count += cstring_write(buffer, 4096, "HTTP/1.1 200 OK\r\n", count);
        count += cstring_write(buffer, 4096, "Server: Webs/1.0.0 (Fudge)\r\n", count);
        count += cstring_write(buffer, 4096, "Content-Type: text/html\r\n", count);
        count += cstring_write_fmt(buffer, 4096, count, "Content-Length: %u\r\n\r\n", &record.size);

        socket_send_tcp(0, ethernet, &local, remote, &router, count, buffer);

        for (offset = 0; (count = fs_read(1, target, id, buffer, 4096, offset)); offset += count)
            socket_send_tcp(0, ethernet, &local, remote, &router, count, buffer);

    }

    else
    {

        count += cstring_write(buffer, 4096, "HTTP/1.1 404 Not Found\r\n", count);
        count += cstring_write(buffer, 4096, "Server: Webs/1.0.0 (Fudge)\r\n", count);
        count += cstring_write(buffer, 4096, "Content-Length: 0\r\n\r\n", count);

        socket_send_tcp(0, ethernet, &local, remote, &router, count, buffer);

    }

}

static void handlehttppacket(unsigned int ethernet, unsigned int source, struct socket *remote)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[4096];
        unsigned int count = ring_read(&input, buffer, newline);

        channel_send(0, source, EVENT_DATA, count, buffer);

        if (count > 4 && buffer_match(buffer, "GET ", 4))
        {

            unsigned int end = buffer_lastbyte(buffer, 4096, ' ');

            cstring_write_zero(request, 128, buffer_write(request, 128, buffer + 4, end - 4 - 1, 0));

        }

        if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
            sendresponse(ethernet, source, remote);

    }

}

static void onmain(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));

    if (ethernet)
    {

        char address[32];
        struct mtwist_state state;
        struct message m;

        mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));
        socket_bind_ipv4s(&router, system_address("router-address", "route", address, 32));
        socket_bind_ipv4s(&local, system_address("local-address", "address", address, 32));
        socket_bind_tcpv(&local, option_getdecimal("local-port"), mtwist_rand(&state), mtwist_rand(&state));
        socket_resolvelocal(0, ethernet, &local);
        channel_send(0, ethernet, EVENT_LINK, 0, 0);
        socket_resolveremote(0, ethernet, &local, &router);
        socket_listen_tcp(ethernet, &local, remotes, 64, &router);

        while (channel_poll(0, ethernet, EVENT_DATA, &m))
        {

            struct socket *remote;

            remote = socket_accept_arp(&local, remotes, 64, m.length, m.data);

            if (remote)
            {

                socket_handle_arp(0, ethernet, &local, remote, m.length, m.data);

            }

            remote = socket_accept_tcp(&local, remotes, 64, m.length, m.data);

            if (remote)
            {

                unsigned char buffer[4096];
                unsigned int count = socket_handle_tcp(0, ethernet, &local, remote, &router, m.length, m.data, 4096, buffer);

                if (count)
                {

                    if (ring_write(&input, buffer, count))
                        handlehttppacket(ethernet, message->source, remote);

                }

            }

        }

        channel_send(0, ethernet, EVENT_UNLINK, 0, 0);

    }

}

void init(void)
{

    unsigned int i;

    ring_init(&input, 4096, inputdata);
    socket_init(&router);
    socket_init(&local);

    for (i = 0; i < 64; i++)
        socket_init(&remotes[i]);

    option_add("clock-service", "clock");
    option_add("ethernet-service", "ethernet");
    option_add("local-address", "");
    option_add("local-port", "80");
    option_add("router-address", "");
    channel_bind(EVENT_MAIN, onmain);

}

