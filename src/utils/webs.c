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

static void sendresponse(struct socket *remote)
{

    char buffer[4096];
    unsigned int count = 0;

    if (!call_walk_absolute(FILE_L0, "/data/html"))
        PANIC();

    if (cstring_length(request) == 0)
        PANIC();

    if (cstring_length(request) == 1 && request[0] == '/')
        cstring_write_zero(request, 128, cstring_write(request, 128, "/index.html", 0));

    if (call_walk_relative(FILE_L1, FILE_L0, request + 1))
    {

        char file[4096];
        unsigned int filesize = call_read(FILE_L1, file, 4096, 0);

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

    socket_send_tcp(FILE_G0, &local, remote, &router, count, buffer);

}

static void handlehttppacket(struct socket *remote)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[4096];
        unsigned int count = ring_read(&input, buffer, newline);

        channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

        if (count > 4 && buffer_match(buffer, "GET ", 4))
        {

            unsigned int end = buffer_lastbyte(buffer, 4096, ' ');

            cstring_write_zero(request, 128, buffer_write(request, 128, buffer + 4, end - 4 - 1, 0));

        }

        if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
            sendresponse(remote);

    }

}

static void seed(struct mtwist_state *state)
{

    struct ctrl_clocksettings settings;

    if (!call_walk_absolute(FILE_L0, option_getstring("clock")))
        PANIC();

    if (!call_walk_relative(FILE_L1, FILE_L0, "ctrl"))
        PANIC();

    call_read_all(FILE_L1, &settings, sizeof (struct ctrl_clocksettings), 0);
    mtwist_seed1(state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

}

static void setupnetwork(struct mtwist_state *state)
{

    if (!call_walk_absolute(FILE_L0, option_getstring("ethernet")))
        PANIC();

    if (!call_walk_relative(FILE_L1, FILE_L0, "addr"))
        PANIC();

    if (!call_walk_relative(FILE_G0, FILE_L0, "data"))
        PANIC();

    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, option_getdecimal("local-port"), mtwist_rand(state), mtwist_rand(state));
    socket_resolvelocal(FILE_L1, &local);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct mtwist_state state;
    struct message message;
    char data[MESSAGE_SIZE];

    seed(&state);
    setupnetwork(&state);
    call_link(FILE_G0, 8000);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_listen_tcp(FILE_G0, &local, remotes, 64, &router);

    while (channel_poll_any(EVENT_DATA, &message, data))
    {

        struct socket *remote;

        remote = socket_accept_arp(&local, remotes, 64, message_datasize(&message), data);

        if (remote)
        {

            socket_handle_arp(FILE_G0, &local, remote, message_datasize(&message), data);

        }

        remote = socket_accept_tcp(&local, remotes, 64, message_datasize(&message), data);

        if (remote)
        {

            unsigned char buffer[4096];
            unsigned int count = socket_handle_tcp(FILE_G0, &local, remote, &router, message_datasize(&message), data, 4096, buffer);

            if (count)
            {

                if (ring_write(&input, buffer, count))
                    handlehttppacket(remote);

            }

        }

    }

    call_unlink(FILE_G0);

}

void init(void)
{

    unsigned int i;

    ring_init(&input, 4096, inputdata);
    socket_init(&router);
    socket_init(&local);

    for (i = 0; i < 64; i++)
        socket_init(&remotes[i]);

    option_add("clock", "system:clock/if.0");
    option_add("ethernet", "system:ethernet/if.0");
    option_add("local-address", "10.0.5.1");
    option_add("local-port", "80");
    option_add("router-address", "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);

}

