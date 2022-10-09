#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputdata[BUFFER_SIZE];
static struct ring input;
static unsigned int isbody;

static void handlehttppacket(void)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count = ring_read(&input, buffer, newline);

        if (isbody)
        {

            channel_sendbuffer(EVENT_DATA, count, buffer);
            file_notify(FILE_G0, EVENT_WMRENDERDATA, count, buffer);

        }

        else
        {

            if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
                isbody = 1;

        }

    }

}

static unsigned int buildrequest(unsigned int count, void *buffer, struct url *kurl)
{

    unsigned int offset = 0;

    offset += cstring_write(buffer, count, "GET /", offset);

    if (kurl->path)
        offset += cstring_write(buffer, count, kurl->path, offset);

    offset += cstring_write(buffer, count, " HTTP/1.1\r\n", offset);
    offset += cstring_write(buffer, count, "Host: ", offset);
    offset += cstring_write(buffer, count, kurl->host, offset);
    offset += cstring_write(buffer, count, "\r\n\r\n", offset);

    return offset;

}

static void resolve(char *domain)
{

    unsigned int id = file_spawn("/bin/dns");
    struct message message;

    if (!id)
        channel_error("Could not spawn process");

    message_init(&message, EVENT_OPTION);
    message_putstringz(&message, "domain");
    message_putstringz(&message, domain);
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
                socket_bind_ipv4s(&remote, value);

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, "system:service/wm"))
        channel_warning("Could not open window manager service");

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *url = option_getstring("url");
    char urldata[BUFFER_SIZE];
    struct url kurl;
    unsigned char buffer[BUFFER_SIZE];
    unsigned int count;
    struct ctrl_clocksettings settings;
    struct mtwist_state state;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        channel_error("Could not find clock device");

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        channel_error("Could not find clock device ctrl");

    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings));
    mtwist_seed1(&state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        channel_error("Could not find ethernet device");

    if (!file_walk(FILE_L1, FILE_L0, "data"))
        channel_error("Could not find ethernet device data");

    if (!file_walk(FILE_L2, FILE_L0, "addr"))
        channel_error("Could not find ethernet device addr");

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_tcpv(&remote, option_getdecimal("remote-port"), 0);
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(FILE_L2, &local);

    if (cstring_length(url) >= 4 && buffer_match(url, "http", 4))
        url_parse(&kurl, urldata, BUFFER_SIZE, url, URL_SCHEME);
    else
        url_parse(&kurl, urldata, BUFFER_SIZE, url, URL_HOST);

    if (kurl.host)
        resolve(kurl.host);

    if (kurl.port)
        socket_bind_tcps(&remote, kurl.port, 0);

    count = buildrequest(BUFFER_SIZE, buffer, &kurl);

    file_link(FILE_L1);
    socket_resolveremote(FILE_L1, &local, &router);
    socket_connect_tcp(FILE_L1, &local, &remote, &router);
    socket_send_tcp(FILE_L1, &local, &remote, &router, count, buffer);

    while ((count = socket_receive_tcp(FILE_L1, &local, &remote, &router, buffer, BUFFER_SIZE)))
    {

        if (ring_write(&input, buffer, count))
            handlehttppacket();

    }

    socket_disconnect_tcp(FILE_L1, &local, &remote, &router);
    file_unlink(FILE_L1);

}

void init(void)
{

    ring_init(&input, BUFFER_SIZE, inputdata);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock", "system:clock/if:0");
    option_add("ethernet", "system:ethernet/if:0");
    option_add("local-address", "10.0.5.1");
    option_add("remote-port", "80");
    option_add("router-address", "10.0.5.80");
    option_add("url", "");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMINIT, onwminit);

}

