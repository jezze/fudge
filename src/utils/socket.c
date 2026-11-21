#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

static void settimeout(unsigned int timer, unsigned int ms)
{

    struct message_timerrequest timerrequest;

    timerrequest.id = 0;
    timerrequest.ms = ms;
    timerrequest.repeat = 0;

    channel_send(0, timer, MESSAGE_TYPE_TIMERREQUEST, sizeof (struct message_timerrequest), &timerrequest);

}

static void ontimerexpire(struct message *message)
{

    channel_close(0);

}

static void onqueryrequest(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));
    unsigned int timer = channel_lookup(option_getstring("timer-service"));
    unsigned int timeout = option_getdecimal("timeout");

    if (ethernet)
    {

        char *optmode = option_getstring("mode");
        unsigned char buffer[MESSAGE_SIZE];
        unsigned int count;

        if (timer)
            settimeout(timer, timeout);

        socket_resolveremote(0, ethernet, &local, &router);

        if (cstring_match(optmode, "tcp"))
        {

            socket_connect_tcp(0, ethernet, &local, &remote, &router);
            socket_send_tcp(0, ethernet, &local, &remote, &router, message->length, message->data);

            while (remote.info.tcp.state == TCP_STATE_ESTABLISHED && (count = socket_receive(0, ethernet, &local, &remote, 1, &router, buffer, MESSAGE_SIZE)))
            {

                channel_send(0, message->source, MESSAGE_TYPE_DATA, count, buffer);

                if (timer)
                    settimeout(timer, timeout);

            }

        }

        if (cstring_match(optmode, "udp"))
        {

            socket_send_udp(0, ethernet, &local, &remote, &router, message->length, message->data);

            count = socket_receive(0, ethernet, &local, &remote, 1, &router, buffer, MESSAGE_SIZE);

            channel_send(0, message->source, MESSAGE_TYPE_DATA, count, buffer);

        }

        if (timer)
            settimeout(timer, 0);

    }

    channel_close(0);

}

static void onmain(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));

    if (ethernet)
    {

        char address[32];
        struct mtwist_state state;

        mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));
        socket_bind_ipv4s(&local, system_address(option_getstring("ethernet-service"), "local-address", "address", address, 32));
        socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&remote, option_getstring("remote-address"));
        socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&router, system_address(option_getstring("ethernet-service"), "router-address", "route", address, 32));
        socket_resolvelocal(0, ethernet, &local);
        channel_send(0, ethernet, MESSAGE_TYPE_LINK, 0, 0);
        channel_send(0, message->source, MESSAGE_TYPE_READY, 0, 0);
        channel_hold(0);
        channel_send(0, ethernet, MESSAGE_TYPE_UNLINK, 0, 0);

    }

    else
    {

        channel_close(0);

    }

}

void init(void)
{

    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock-service", "clock:0");
    option_add("ethernet-service", "ethernet:0");
    option_add("local-address", "");
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "");
    option_add("mode", "");
    option_add("timer-service", "timer:0");
    option_add("timeout", "10000");
    channel_bind(MESSAGE_TYPE_QUERYREQUEST, onqueryrequest);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_TIMEREXPIRE, ontimerexpire);

}

