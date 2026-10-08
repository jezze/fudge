#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputdata[4096];
static struct ring input;
static unsigned int isbody;

static unsigned int buildrequest(unsigned int count, void *buffer, struct url *url)
{

    return cstring_write_fmt(buffer, count, 0, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", (url->path) ? url->path : "", url->host);

}

static void handlehttppacket(unsigned int wm)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[4096];
        unsigned int count = ring_read(&input, buffer, newline);

        if (isbody)
            channel_send(0, wm, EVENT_WMRENDERDATA, count, buffer);
        else if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
            isbody = 1;

    }

}

static void dnsresolve(struct socket *socket, char *domain)
{

    char address[32];

    if (system_resolve(domain, address, 32))
        socket_bind_ipv4s(socket, address);

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

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(0, wm, EVENT_WMMAP, 0, 0);
        channel_hold(0);
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwminit(struct message *message)
{

    unsigned int ethernet = channel_lookup(option_getstring("ethernet-service"));

    if (ethernet)
    {

        char address[32];
        char urldata[4096];
        struct url url;
        unsigned char buffer[4096];
        unsigned int count;
        struct mtwist_state state;

        mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));
        socket_bind_ipv4s(&local, system_address("local-address", "address", address, 32));
        socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&remote, option_getstring("remote-address"));
        socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));
        socket_bind_ipv4s(&router, system_address("router-address", "route", address, 32));
        socket_resolvelocal(0, ethernet, &local);
        parseurl(&url, urldata, 4096);

        if (url.host)
            dnsresolve(&remote, url.host);

        if (url.port)
            socket_bind_tcps(&remote, url.port, mtwist_rand(&state), mtwist_rand(&state));

        channel_send(0, ethernet, EVENT_LINK, 0, 0);
        socket_resolveremote(0, ethernet, &local, &router);
        socket_connect_tcp(0, ethernet, &local, &remote, &router);
        socket_send_tcp(0, ethernet, &local, &remote, &router, buildrequest(4096, buffer, &url), buffer);

        while ((count = socket_receive(0, ethernet, &local, &remote, 1, &router, buffer, 4096)))
        {

            if (ring_write(&input, buffer, count))
                handlehttppacket(message->source);

        }

        channel_send(0, ethernet, EVENT_UNLINK, 0, 0);

    }

}

void init(void)
{

    ring_init(&input, 4096, inputdata);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("wm-service", "wm");
    option_add("clock-service", "clock");
    option_add("ethernet-service", "ethernet");
    option_add("local-address", "");
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "");
    option_add("url", "");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

}

