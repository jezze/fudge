#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputbuffer[BUFFER_SIZE];
static struct ring input;

static unsigned int buildrequest(unsigned int count, void *buffer)
{

    return cstring_writefmt4(buffer, count, "NICK %s\nUSER %s 0 * :%s\nJOIN %s\n", 0, option_getstring("nick"), option_getstring("nick"), option_getstring("realname"), option_getstring("channel"));

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

        char outputdata[BUFFER_SIZE];

        socket_send_tcp(FILE_G0, &local, &remote, &router, cstring_writefmt3(outputdata, BUFFER_SIZE, "PRIVMSG %s :%w", 0, option_getstring("channel"), buffer, &count), outputdata);

    }

}

static void dnsresolve(struct socket *socket, char *domain)
{

    unsigned int id = file_spawn(FILE_L0, "/bin/dns");

    if (id)
    {

        char data[MESSAGE_SIZE];
        unsigned int count;

        channel_listen(id, EVENT_QUERY);
        channel_listen(id, EVENT_CLOSE);
        channel_sendfmt1(id, EVENT_OPTION, "domain\\0%s\\0", domain);
        channel_send(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, EVENT_QUERY, data)))
        {

            char *key = cstring_tindex(data, count, 0);
            char *value = cstring_tindex(data, count, 1);

            if (cstring_match(key, "data"))
                socket_bind_ipv4s(socket, value);

        }

    }

    else
    {

        channel_error("Could not spawn process");

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

static void setupnetwork(struct mtwist_state *state)
{

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        channel_error("Could not find ethernet device");

    if (!file_walk(FILE_L1, FILE_L0, "addr"))
        channel_error("Could not find ethernet device addr");

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        channel_error("Could not find ethernet device data");

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
    char buffer[BUFFER_SIZE];
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
        channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, BUFFER_SIZE);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);
    setupnetwork(&state);

    if (cstring_length(option_getstring("domain")))
        dnsresolve(&remote, option_getstring("domain"));

    file_link(FILE_G0);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_connect_tcp(FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, buildrequest(BUFFER_SIZE, buffer), buffer);

    while ((count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, BUFFER_SIZE)))
        channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    ring_init(&input, BUFFER_SIZE, inputbuffer);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock", "system:clock/if:0");
    option_add("ethernet", "system:ethernet/if:0");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "6667");
    option_add("router-address", "10.0.5.80");
    option_add("domain", "irc.libera.chat");
    option_add("channel", "#fudge");
    option_add("nick", "");
    option_add("realname", "Anonymous User");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);

}

