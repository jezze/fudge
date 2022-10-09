#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputdata[BUFFER_SIZE];
static struct ring input;
static char *response =
    "<html>\n"
    "  <h1>Hello world!</h1>\n"
    "  <p>This server is running on Fudge</p>\n"
    "</html>\n";

static void sendresponse(void)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += cstring_write(buffer, BUFFER_SIZE, "HTTP/1.1 200 OK\r\n", count);
    count += cstring_write(buffer, BUFFER_SIZE, "Server: Webs/1.0.0 (Fudge)\r\n", count);
    count += cstring_write(buffer, BUFFER_SIZE, "Content-Type: text/html\r\n", count);
    count += cstring_write(buffer, BUFFER_SIZE, "Content-Length: ", count);
    count += cstring_writevalue(buffer, BUFFER_SIZE, cstring_length(response), 10, 0, count);
    count += cstring_write(buffer, BUFFER_SIZE, "\r\n", count);
    count += cstring_write(buffer, BUFFER_SIZE, "\r\n", count);
    count += cstring_write(buffer, BUFFER_SIZE, response, count);

    socket_send_tcp(FILE_G0, &local, &remote, &router, count, buffer);

}

static void handlehttppacket(void)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count = ring_read(&input, buffer, newline);

        channel_sendbuffer(EVENT_DATA, count, buffer);

        if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
            sendresponse();

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

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        channel_error("Could not find ethernet device data");

    if (!file_walk(FILE_L2, FILE_L0, "addr"))
        channel_error("Could not find ethernet device addr");

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, option_getdecimal("local-port"), mtwist_rand(&state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(FILE_L2, &local);
    file_link(FILE_G0);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_listen_tcp(FILE_G0, &local, &remote, &router);

    while ((count = socket_receive_tcp(FILE_G0, &local, &remote, &router, buffer, BUFFER_SIZE)))
    {

        if (ring_write(&input, buffer, count))
            handlehttppacket();

    }

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    ring_init(&input, BUFFER_SIZE, inputdata);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock", "system:clock/if:0");
    option_add("ethernet", "system:ethernet/if:0");
    option_add("local-address", "10.0.5.1");
    option_add("local-port", "80");
    option_add("router-address", "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);

}

