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

static void sendresponse(unsigned int source, struct socket *remote)
{

    unsigned int service = fsp_auth(option_getstring("initrd:"));
    unsigned int root = fsp_walk(service, 0, "data/html");
    unsigned int id;
    char buffer[4096];
    unsigned int count = 0;

    if (cstring_length(request) == 1 && request[0] == '/')
        cstring_write_zero(request, 128, cstring_write(request, 128, "/index.html", 0));

    id = fsp_walk(service, root, request + 1);

    if (id)
    {

        char file[4096];
        unsigned int filesize = fsp_read_full(service, id, file, 4096, 0);

        count += cstring_write(buffer, 4096, "HTTP/1.1 200 OK\r\n", count);
        count += cstring_write(buffer, 4096, "Server: Webs/1.0.0 (Fudge)\r\n", count);
        count += cstring_write(buffer, 4096, "Content-Type: text/html\r\n", count);
        count += cstring_write_fmt1(buffer, 4096, "Content-Length: %u\r\n\r\n", count, &filesize);
        count += buffer_write(buffer, 4096, file, filesize, count);

    }

    else
    {

        count += cstring_write(buffer, 4096, "HTTP/1.1 404 Not Found\r\n", count);
        count += cstring_write(buffer, 4096, "Server: Webs/1.0.0 (Fudge)\r\n", count);
        count += cstring_write(buffer, 4096, "Content-Length: 0\r\n\r\n", count);

    }

    socket_send_tcp(option_getdecimal("ethernet-service"), &local, remote, &router, count, buffer);

}

static void handlehttppacket(unsigned int source, struct socket *remote)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[4096];
        unsigned int count = ring_read(&input, buffer, newline);

        channel_send_buffer(source, EVENT_DATA, count, buffer);

        if (count > 4 && buffer_match(buffer, "GET ", 4))
        {

            unsigned int end = buffer_lastbyte(buffer, 4096, ' ');

            cstring_write_zero(request, 128, buffer_write(request, 128, buffer + 4, end - 4 - 1, 0));

        }

        if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
            sendresponse(source, remote);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int ethernetservice = fsp_auth(option_getstring("ethernet"));
    unsigned int ethernetaddr = fsp_walk(ethernetservice, fsp_walk(ethernetservice, 0, option_getstring("ethernet")), "addr");
    struct ctrl_clocksettings settings;
    struct mtwist_state state;
    struct message message;
    char data[MESSAGE_SIZE];

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(EVENT_CLOCKINFO, sizeof (struct ctrl_clocksettings), &settings);
    mtwist_seed1(&state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, option_getdecimal("local-port"), mtwist_rand(&state), mtwist_rand(&state));
    socket_resolvelocal(ethernetservice, ethernetaddr, &local);
    channel_send(option_getdecimal("ethernet-service"), EVENT_LINK);
    socket_resolveremote(option_getdecimal("ethernet-service"), &local, &router);
    socket_listen_tcp(option_getdecimal("ethernet-service"), &local, remotes, 64, &router);

    while (channel_poll(EVENT_DATA, &message, MESSAGE_SIZE, data))
    {

        struct socket *remote;

        remote = socket_accept_arp(&local, remotes, 64, message_datasize(&message), data);

        if (remote)
        {

            socket_handle_arp(option_getdecimal("ethernet-service"), &local, remote, message_datasize(&message), data);

        }

        remote = socket_accept_tcp(&local, remotes, 64, message_datasize(&message), data);

        if (remote)
        {

            unsigned char buffer[4096];
            unsigned int count = socket_handle_tcp(option_getdecimal("ethernet-service"), &local, remote, &router, message_datasize(&message), data, 4096, buffer);

            if (count)
            {

                if (ring_write(&input, buffer, count))
                    handlehttppacket(source, remote);

            }

        }

    }

    channel_send(option_getdecimal("ethernet-service"), EVENT_UNLINK);

}

void init(void)
{

    unsigned int i;

    ring_init(&input, 4096, inputdata);
    socket_init(&router);
    socket_init(&local);

    for (i = 0; i < 64; i++)
        socket_init(&remotes[i]);

    option_add("clock-service", "220");
    option_add("ethernet-service", "108");
    option_add("local-address", "10.0.5.1");
    option_add("local-port", "80");
    option_add("router-address", "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);

}

