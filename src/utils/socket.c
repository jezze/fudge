#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

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

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(state), mtwist_rand(state), mtwist_rand(state));
    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(state), mtwist_rand(state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(FILE_L1, &local);

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    char *optmode = option_getstring("mode");
    unsigned char buffer[MESSAGE_SIZE];
    unsigned int count;

    socket_resolveremote(FILE_G0, &local, &router);

    if (cstring_match(optmode, "tcp"))
    {

        socket_connect_tcp(FILE_G0, &local, &remote, &router);
        socket_send_tcp(FILE_G0, &local, &remote, &router, msize, mdata);

        while ((count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, MESSAGE_SIZE)))
            channel_send_buffer(source, EVENT_DATA, count, buffer);

    }

    if (cstring_match(optmode, "udp"))
    {

        socket_send_udp(FILE_G0, &local, &remote, &router, msize, mdata);

        count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, MESSAGE_SIZE);

        channel_send_buffer(source, EVENT_DATA, count, buffer);

    }

    channel_send(source, EVENT_CLOSE);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct mtwist_state state;

    seed(&state);
    setupnetwork(&state);
    call_link(FILE_G0, 8000);

    while (channel_process());

    call_unlink(FILE_G0);

}

void init(void)
{

    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock", "system:clock/if.0");
    option_add("ethernet", "system:ethernet/if.0");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "10.0.5.80");
    option_add("mode", "");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

