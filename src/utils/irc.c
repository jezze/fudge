#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputbuffer[4096];
static struct ring input;

static unsigned int buildrequest(unsigned int count, void *buffer)
{

    return cstring_write_fmt(buffer, count, 0, "NICK %s\nUSER %s 0 * :%s\nJOIN %s\n", option_getstring("nick"), option_getstring("nick"), option_getstring("realname"), option_getstring("channel"));

}

static void interpret(unsigned int ethernet, void *buffer, unsigned int count)
{

    char *data = buffer;

    if (data[0] == '/')
    {

        socket_send_tcp(0, ethernet, &local, &remote, &router, count - 1, data + 1);

    }

    else
    {

        char outputdata[4096];

        socket_send_tcp(0, ethernet, &local, &remote, &router, cstring_write_fmt(outputdata, 4096, 0, "PRIVMSG %s :%w", option_getstring("channel"), buffer, &count), outputdata);

    }

}

static void dnsresolve(struct socket *socket, char *domain)
{

    char address[32];

    if (system_resolve(domain, address, 32))
        socket_bind_ipv4s(socket, address);

}

static void onconsoledata(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));

    if (ethernet)
    {

        struct event_consoledata *consoledata = message->data;
        char buffer[4096];
        unsigned int count;

        if (!remote.resolved)
            return;

        switch (consoledata->data)
        {

        case '\0':
        case '\f':
        case '\t':
        case '\b':
        case 0x7F:
            break;

        case '\r':
            consoledata->data = '\n';

        default:
            ring_write(&input, &consoledata->data, 1);
            channel_send(0, 0 /* TODO: Should not be 0 */, EVENT_DATA, 1, &consoledata->data);

            if (consoledata->data == '\n' && (count = ring_read(&input, buffer, 4096)))
                interpret(ethernet, buffer, count);

            break;

        }

    }

}

static void onmain(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));

    if (ethernet)
    {

        char address[32];
        char buffer[4096];
        unsigned int count;
        struct mtwist_state state;

        mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));
        socket_bind_ipv4s(&local, system_address("local-address", "address", address, 32));
        socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&remote, option_getstring("remote-address"));
        socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&router, system_address("router-address", "route", address, 32));
        socket_resolvelocal(0, ethernet, &local);

        if (cstring_length(option_getstring("domain")))
            dnsresolve(&remote, option_getstring("domain"));

        channel_send(0, ethernet, EVENT_LINK, 0, 0);
        socket_resolveremote(0, ethernet, &local, &router);
        socket_connect_tcp(0, ethernet, &local, &remote, &router);
        socket_send_tcp(0, ethernet, &local, &remote, &router, buildrequest(4096, buffer), buffer);

        while ((count = socket_receive(0, ethernet, &local, &remote, 1, &router, buffer, 4096)))
            channel_send(0, message->source, EVENT_DATA, count, buffer);

        channel_send(0, ethernet, EVENT_UNLINK, 0, 0);

    }

}

void init(void)
{

    ring_init(&input, 4096, inputbuffer);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock-service", "clock");
    option_add("ethernet-service", "ethernet");
    option_add("local-address", "");
    option_add("remote-address", "");
    option_add("remote-port", "6667");
    option_add("router-address", "");
    option_add("domain", "irc.libera.chat");
    option_add("channel", "#fudge");
    option_add("nick", "");
    option_add("realname", "Anonymous User");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);

}

