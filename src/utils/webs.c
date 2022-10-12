#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket router;
static struct socket local;
static struct socket remotes[64];
static char inputdata[BUFFER_SIZE];
static struct ring input;
static char request[128];

static void sendresponse(struct socket *remote)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    if (!file_walk2(FILE_L0, "/data/html"))
        channel_error("Could not find html root directory");

    if (cstring_length(request) == 0)
        channel_error("Incorrect request");

    if (cstring_length(request) == 1 && request[0] == '/')
        cstring_writezero(request, 128, cstring_write(request, 128, "/index.html", 0));

    if (file_walk(FILE_L1, FILE_L0, request + 1))
    {

        char file[BUFFER_SIZE];
        unsigned int filesize = file_read(FILE_L1, file, BUFFER_SIZE);

        count += cstring_write(buffer, BUFFER_SIZE, "HTTP/1.1 200 OK\r\n", count);
        count += cstring_write(buffer, BUFFER_SIZE, "Server: Webs/1.0.0 (Fudge)\r\n", count);
        count += cstring_write(buffer, BUFFER_SIZE, "Content-Type: text/html\r\n", count);
        count += cstring_write(buffer, BUFFER_SIZE, "Content-Length: ", count);
        count += cstring_writevalue(buffer, BUFFER_SIZE, filesize, 10, 0, count);
        count += cstring_write(buffer, BUFFER_SIZE, "\r\n", count);
        count += cstring_write(buffer, BUFFER_SIZE, "\r\n", count);
        count += buffer_write(buffer, BUFFER_SIZE, file, filesize, count);

    }

    else
    {

        count += cstring_write(buffer, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\n", count);
        count += cstring_write(buffer, BUFFER_SIZE, "Server: Webs/1.0.0 (Fudge)\r\n", count);
        count += cstring_write(buffer, BUFFER_SIZE, "Content-Length: 0\r\n", count);
        count += cstring_write(buffer, BUFFER_SIZE, "\r\n", count);

    }

    socket_send_tcp(FILE_G0, &local, remote, &router, count, buffer);

}

static void handlehttppacket(struct socket *remote)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count = ring_read(&input, buffer, newline);

        channel_sendbuffer(EVENT_DATA, count, buffer);

        if (count > 4 && buffer_match(buffer, "GET ", 4))
        {

            unsigned int end = buffer_lastbyte(buffer, BUFFER_SIZE, ' ');

            cstring_writezero(request, 128, buffer_write(request, 128, buffer + 4, end - 4 - 1, 0));

        }

        if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
            sendresponse(remote);

    }

}

static void seed(struct mtwist_state *state)
{

    struct ctrl_clocksettings settings;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        channel_error("Could not find clock device");

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        channel_error("Could not find clock device ctrl");

    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings));
    mtwist_seed1(state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char buffer[BUFFER_SIZE];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        channel_error("Could not find ethernet device");

    if (!file_walk(FILE_L1, FILE_L0, "addr"))
        channel_error("Could not find ethernet device addr");

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        channel_error("Could not find ethernet device data");

    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, option_getdecimal("local-port"), mtwist_rand(&state), mtwist_rand(&state));
    socket_resolvelocal(FILE_L1, &local);
    file_link(FILE_G0);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_listen_tcp(FILE_G0, &local, remotes, 64, &router);

    while ((count = socket_receive(FILE_G0, &local, remotes, 64, &router, buffer, BUFFER_SIZE)))
    {

        if (ring_write(&input, buffer, count))
            handlehttppacket(&remotes[0]);

    }

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    unsigned int i;

    ring_init(&input, BUFFER_SIZE, inputdata);
    socket_init(&router);
    socket_init(&local);

    for (i = 0; i < 64; i++)
        socket_init(&remotes[i]);

    option_add("clock", "system:clock/if:0");
    option_add("ethernet", "system:ethernet/if:0");
    option_add("local-address", "10.0.5.1");
    option_add("local-port", "80");
    option_add("router-address", "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);

}

