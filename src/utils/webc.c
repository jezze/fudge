#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputdata[BUFFER_SIZE];
static struct ring input;

static unsigned int buildrequest(unsigned int count, void *buffer, struct url *url)
{

    return cstring_writefmt2(buffer, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", 0, (url->path) ? url->path : "", url->host);

}

static void handlehttppacket(void)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count;

    while ((count = ring_read(&input, buffer, MESSAGE_SIZE)))
        channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

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

static void parseurl(struct url *url, char *urldata, unsigned int urlsize)
{

    char *opturl = option_getstring("url");
    unsigned int count = cstring_length(opturl);

    if (count)
    {

        if (cstring_length(opturl) >= 4 && buffer_match(opturl, "http", 4))
            url_parse(url, urldata, urlsize, opturl, URL_SCHEME);
        else
            url_parse(url, urldata, urlsize, opturl, URL_HOST);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char urldata[BUFFER_SIZE];
    struct url url;
    unsigned char buffer[BUFFER_SIZE];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);
    setupnetwork(&state);
    parseurl(&url, urldata, BUFFER_SIZE);

    if (url.host)
        dnsresolve(&remote, url.host);

    if (url.port)
        socket_bind_tcps(&remote, url.port, mtwist_rand(&state), mtwist_rand(&state));

    file_link(FILE_G0);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_connect_tcp(FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, buildrequest(BUFFER_SIZE, buffer, &url), buffer);

    while ((count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, BUFFER_SIZE)))
    {

        if (ring_write(&input, buffer, count))
            handlehttppacket();

    }

    file_unlink(FILE_G0);
    channel_close();

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
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "10.0.5.80");
    option_add("url", "");
    channel_bind(EVENT_MAIN, onmain);

}

