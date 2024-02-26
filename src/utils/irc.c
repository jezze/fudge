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

        socket_send_tcp(FILE_G0, &local, &remote, &router, count - 1, data + 1);

    }

    else
    {

        char outputdata[4096];

        socket_send_tcp(FILE_G0, &local, &remote, &router, cstring_write_fmt3(outputdata, 4096, "PRIVMSG %s :%w", 0, option_getstring("channel"), buffer, &count), outputdata);

    }

}

static void dnsresolve(struct socket *socket, char *domain)
{

    unsigned int channel = call_spawn_absolute(FILE_L0, FILE_PW, option_getstring("dns"));

    if (channel)
    {

        char data[MESSAGE_SIZE];
        unsigned int count;

        channel_listen(channel, EVENT_QUERY);
        channel_listen(channel, EVENT_CLOSE);
        channel_send_fmt1(channel, EVENT_OPTION, "domain\\0%s\\0", domain);
        channel_send(channel, EVENT_MAIN);

        while ((count = channel_read_from(channel, EVENT_QUERY, data)))
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

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", option_getstring("dns"));

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

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(state), mtwist_rand(state), mtwist_rand(state));
    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(state), mtwist_rand(state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(FILE_L1, &local);

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
        channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, 4096);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[4096];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);
    setupnetwork(&state);

    if (cstring_length(option_getstring("domain")))
        dnsresolve(&remote, option_getstring("domain"));

    call_link(FILE_G0, 8000);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_connect_tcp(FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, buildrequest(4096, buffer), buffer);

    while ((count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, 4096)))
        channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

    call_unlink(FILE_G0);

}

void init(void)
{

    ring_init(&input, 4096, inputbuffer);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock", "system:clock/if.0");
    option_add("ethernet", "system:ethernet/if.0");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "6667");
    option_add("router-address", "10.0.5.80");
    option_add("domain", "irc.libera.chat");
    option_add("channel", "#fudge");
    option_add("nick", "");
    option_add("realname", "Anonymous User");
    option_add("dns", "initrd:/bin/dns");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);

}

