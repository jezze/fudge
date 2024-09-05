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

    return cstring_write_fmt4(buffer, count, "NICK %s\nUSER %s 0 * :%s\nJOIN %s\n", 0, option_getstring("nick"), option_getstring("nick"), option_getstring("realname"), option_getstring("channel"));

}

static void interpret(void *buffer, unsigned int count)
{

    char *data = buffer;

    if (data[0] == '/')
    {

        socket_send_tcp(option_getdecimal("ethernet-service"), &local, &remote, &router, count - 1, data + 1);

    }

    else
    {

        char outputdata[4096];

        socket_send_tcp(option_getdecimal("ethernet-service"), &local, &remote, &router, cstring_write_fmt3(outputdata, 4096, "PRIVMSG %s :%w", 0, option_getstring("channel"), buffer, &count), outputdata);

    }

}

static void dnsresolve(struct socket *socket, char *domain)
{

    unsigned int channel = fsp_spawn(option_getstring("dns"));

    if (channel)
    {

        char data[MESSAGE_SIZE];
        unsigned int count;

        channel_send_fmt1(channel, EVENT_OPTION, "domain\\0%s\\0", domain);
        channel_send(channel, EVENT_MAIN);

        while ((count = channel_read(EVENT_QUERY, MESSAGE_SIZE, data)))
        {

            unsigned int i;
            char *key;

            for (i = 0; (key = buffer_tindex(data, count, '\0', i)); i += 2)
            {

                if (cstring_match(key, "data"))
                {

                    char *value = key + cstring_length_zero(key);

                    socket_bind_ipv4s(socket, value);

                    break;

                }

            }

        }

        channel_send(channel, EVENT_END);
        channel_wait(EVENT_TERMRESPONSE);

    }

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    char buffer[4096];
    unsigned int count;

    if (!remote.resolved)
        return;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\f':
        break;

    case '\t':
        break;

    case '\b':
    case 0x7F:
        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        ring_write(&input, &consoledata->data, 1);
        channel_send_buffer(0 /* TODO: Should not be 0 */, EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, 4096);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_send_buffer(0 /* TODO: Should not be 0 */, EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int ethernetservice = fsp_auth(option_getstring("ethernet"));
    unsigned int ethernetaddr = fsp_walk(ethernetservice, fsp_walk(ethernetservice, 0, option_getstring("ethernet")), "addr");
    char buffer[4096];
    unsigned int count;
    struct ctrl_clocksettings settings;
    struct mtwist_state state;

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(EVENT_CLOCKINFO, sizeof (struct ctrl_clocksettings), &settings);
    mtwist_seed1(&state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(ethernetservice, ethernetaddr, &local);

    if (cstring_length(option_getstring("domain")))
        dnsresolve(&remote, option_getstring("domain"));

    channel_send(option_getdecimal("ethernet-service"), EVENT_LINK);
    socket_resolveremote(option_getdecimal("ethernet-service"), &local, &router);
    socket_connect_tcp(option_getdecimal("ethernet-service"), &local, &remote, &router);
    socket_send_tcp(option_getdecimal("ethernet-service"), &local, &remote, &router, buildrequest(4096, buffer), buffer);

    while ((count = socket_receive(option_getdecimal("ethernet-service"), &local, &remote, 1, &router, buffer, 4096)))
        channel_send_buffer(source, EVENT_DATA, count, buffer);

    channel_send(option_getdecimal("ethernet-service"), EVENT_UNLINK);

}

void init(void)
{

    ring_init(&input, 4096, inputbuffer);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock-service", "220");
    option_add("ethernet-service", "108");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "6667");
    option_add("router-address", "10.0.5.80");
    option_add("domain", "irc.libera.chat");
    option_add("channel", "#fudge");
    option_add("nick", "");
    option_add("realname", "Anonymous User");
    option_add("dns", "initrd:bin/dns");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);

}

