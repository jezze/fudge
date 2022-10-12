#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputbuffer[BUFFER_SIZE];
static struct ring input;

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
        unsigned int offset = 0;

        offset += cstring_write(outputdata, BUFFER_SIZE, "PRIVMSG ", offset);
        offset += cstring_write(outputdata, BUFFER_SIZE, option_getstring("channel"), offset);
        offset += cstring_write(outputdata, BUFFER_SIZE, " :", offset);
        offset += buffer_write(outputdata, BUFFER_SIZE, buffer, count, offset);

        socket_send_tcp(FILE_G0, &local, &remote, &router, offset, outputdata);

    }

}

static unsigned int buildrequest(unsigned int count, void *buffer)
{

    unsigned int offset = 0;

    offset += cstring_write(buffer, count, "NICK ", offset);
    offset += cstring_write(buffer, count, option_getstring("nick"), offset);
    offset += cstring_write(buffer, count, "\n", offset);
    offset += cstring_write(buffer, count, "USER ", offset);
    offset += cstring_write(buffer, count, option_getstring("nick"), offset);
    offset += cstring_write(buffer, count, " 0 * :", offset);
    offset += cstring_write(buffer, count, option_getstring("realname"), offset);
    offset += cstring_write(buffer, count, "\n", offset);
    offset += cstring_write(buffer, count, "JOIN ", offset);
    offset += cstring_write(buffer, count, option_getstring("channel"), offset);
    offset += cstring_write(buffer, count, "\n", offset);

    return offset;

}

static void dnsresolve(void)
{

    unsigned int id = file_spawn("/bin/dns");
    struct message message;

    if (!id)
        channel_error("Could not spawn process");

    message_init(&message, EVENT_OPTION);
    message_putstringz(&message, "domain");
    message_putstringz(&message, option_getstring("domain"));
    channel_redirectback(id, EVENT_QUERY);
    channel_redirectback(id, EVENT_CLOSE);
    channel_sendmessageto(id, &message);
    channel_sendto(id, EVENT_MAIN);

    while (channel_pollfrom(id, &message) != EVENT_CLOSE)
    {

        if (message.header.event == EVENT_QUERY)
        {

            char *key = message.data.buffer;
            char *value = key + cstring_lengthz(key);

            if (cstring_match(key, "data"))
                option_set("remote-address", value);

        }

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
        channel_sendbuffer(EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, BUFFER_SIZE);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_sendbuffer(EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        channel_error("Could not find ethernet device");

    if (!file_walk(FILE_L1, FILE_L0, "addr"))
        channel_error("Could not find ethernet device addr");

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        channel_error("Could not find ethernet device data");

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(FILE_L1, &local);

    if (cstring_length(option_getstring("domain")))
        dnsresolve();

    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_tcps(&remote, option_getstring("remote-port"), mtwist_rand(&state), mtwist_rand(&state));

    count = buildrequest(BUFFER_SIZE, buffer);

    file_link(FILE_G0);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_connect_tcp(FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, count, buffer);

    while ((count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, BUFFER_SIZE)))
        channel_sendbuffer(EVENT_DATA, count, buffer);

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

