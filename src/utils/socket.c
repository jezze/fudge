#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    char *optmode = option_getstring("mode");
    unsigned char buffer[MESSAGE_SIZE];
    unsigned int count;

    socket_resolveremote(option_getdecimal("ethernet-service"), &local, &router);

    if (cstring_match(optmode, "tcp"))
    {

        socket_connect_tcp(option_getdecimal("ethernet-service"), &local, &remote, &router);
        socket_send_tcp(option_getdecimal("ethernet-service"), &local, &remote, &router, msize, mdata);

        while ((count = socket_receive(option_getdecimal("ethernet-service"), &local, &remote, 1, &router, buffer, MESSAGE_SIZE)))
            channel_send_buffer(source, EVENT_DATA, count, buffer);

    }

    if (cstring_match(optmode, "udp"))
    {

        socket_send_udp(option_getdecimal("ethernet-service"), &local, &remote, &router, msize, mdata);

        count = socket_receive(option_getdecimal("ethernet-service"), &local, &remote, 1, &router, buffer, MESSAGE_SIZE);

        channel_send_buffer(source, EVENT_DATA, count, buffer);

    }

    channel_send(source, EVENT_DATA);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_clockinfo clockinfo;
    struct mtwist_state state;

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
    mtwist_seed1(&state, time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(option_getdecimal("ethernet-service"), &local);
    channel_send(option_getdecimal("ethernet-service"), EVENT_LINK);

    while (channel_process());

    channel_send(option_getdecimal("ethernet-service"), EVENT_UNLINK);

}

void init(void)
{

    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock-service", "220");
    option_add("ethernet-service", "108");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "10.0.5.80");
    option_add("mode", "");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

