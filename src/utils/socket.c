#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

static void onqueryrequest(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));

    if (ethernet)
    {

        char *optmode = option_getstring("mode");
        unsigned char buffer[MESSAGE_SIZE];
        unsigned int count;

        socket_resolveremote(0, ethernet, &local, &router);

        if (cstring_match(optmode, "tcp"))
        {

            socket_connect_tcp(0, ethernet, &local, &remote, &router);
            socket_send_tcp(0, ethernet, &local, &remote, &router, message->length, message->data);

            while ((count = socket_receive(0, ethernet, &local, &remote, 1, &router, buffer, MESSAGE_SIZE)))
                channel_send(0, message->source, EVENT_DATA, count, buffer);

        }

        if (cstring_match(optmode, "udp"))
        {

            socket_send_udp(0, ethernet, &local, &remote, &router, message->length, message->data);

            count = socket_receive(0, ethernet, &local, &remote, 1, &router, buffer, MESSAGE_SIZE);

            channel_send(0, message->source, EVENT_DATA, count, buffer);

        }

    }

    channel_close();

}

static void onmain(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));

    if (ethernet)
    {

        struct mtwist_state state;

        mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));
        socket_bind_ipv4s(&local, option_getstring("local-address"));
        socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&remote, option_getstring("remote-address"));
        socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&router, option_getstring("router-address"));
        socket_resolvelocal(0, ethernet, &local);
        channel_send(0, ethernet, EVENT_LINK, 0, 0);
        channel_send(0, message->source, EVENT_READY, 0, 0);
        channel_hold(0);
        channel_send(0, ethernet, EVENT_UNLINK, 0, 0);

    }

    else
    {

        channel_close();

    }

}

void init(void)
{

    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock-service", "clock");
    option_add("ethernet-service", "ethernet");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "10.0.5.80");
    option_add("mode", "");
    channel_bind(EVENT_QUERYREQUEST, onqueryrequest);
    channel_bind(EVENT_MAIN, onmain);

}

