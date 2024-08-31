#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

static void seed(struct mtwist_state *state)
{

    unsigned int service = fsp_auth(option_getstring("clock"));

    if (service)
    {

        unsigned int id = fsp_walk(service, fsp_walk(service, 0, option_getstring("clock")), "ctrl");

        if (id)
        {

            struct ctrl_clocksettings settings;

            fsp_read_all(service, id, &settings, sizeof (struct ctrl_clocksettings), 0);
            mtwist_seed1(state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

        }

    }

}

static void setupnetwork(unsigned int source, struct mtwist_state *state)
{

    unsigned int ethernetservice = fsp_auth(option_getstring("ethernet"));
    unsigned int ethernetaddr = fsp_walk(ethernetservice, fsp_walk(ethernetservice, 0, option_getstring("ethernet")), "addr");

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(state), mtwist_rand(state), mtwist_rand(state));
    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(state), mtwist_rand(state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(ethernetservice, ethernetaddr, &local);

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    char *optmode = option_getstring("mode");
    unsigned char buffer[MESSAGE_SIZE];
    unsigned int count;

    socket_resolveremote(108, &local, &router);

    if (cstring_match(optmode, "tcp"))
    {

        socket_connect_tcp(108, &local, &remote, &router);
        socket_send_tcp(108, &local, &remote, &router, msize, mdata);

        while ((count = socket_receive(108, &local, &remote, 1, &router, buffer, MESSAGE_SIZE)))
            channel_send_buffer(source, EVENT_DATA, count, buffer);

    }

    if (cstring_match(optmode, "udp"))
    {

        socket_send_udp(108, &local, &remote, &router, msize, mdata);

        count = socket_receive(108, &local, &remote, 1, &router, buffer, MESSAGE_SIZE);

        channel_send_buffer(source, EVENT_DATA, count, buffer);

    }

    channel_send(source, EVENT_DATA);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int ethernetservice = fsp_auth(option_getstring("ethernet"));
    unsigned int ethernetdata = fsp_walk(ethernetservice, fsp_walk(ethernetservice, 0, option_getstring("ethernet")), "data");
    struct mtwist_state state;

    seed(&state);
    setupnetwork(source, &state);
    fsp_link(ethernetservice, ethernetdata);

    while (channel_process());

    fsp_unlink(ethernetservice, ethernetdata);

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

