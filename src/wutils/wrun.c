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

    return cstring_write_fmt2(buffer, count, 0, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", (url->path) ? url->path : "", url->host);

}

static void handlehttppacket(unsigned int source)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[4096];
        unsigned int count = ring_read(&input, buffer, newline);

        if (isbody)
        {

            channel_send_buffer(0, source, EVENT_DATA, count, buffer);
            channel_send_buffer(0, option_getdecimal("wm-service"), EVENT_WMRENDERDATA, count, buffer);

        }

        else
        {

            if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
                isbody = 1;

        }

    }

}

static void dnsresolve(struct socket *socket, char *domain)
{

    unsigned int target = fs_spawn(1, option_getstring("dns"));

    if (target)
    {

        struct message message;
        char data[MESSAGE_SIZE];

        channel_send_fmt1(1, target, EVENT_OPTION, "domain=%s\n", domain);
        channel_send(1, target, EVENT_MAIN);

        while (channel_poll(1, target, EVENT_QUERYRESPONSE, &message, MESSAGE_SIZE, data))
        {

            unsigned int i;
            char *key;

            for (i = 0; (key = buffer_tindex(data, message_datasize(&message), '\0', i)); i += 2)
            {

                if (cstring_match(key, "data"))
                {

                    char *value = key + cstring_length_zero(key);

                    socket_bind_ipv4s(socket, value);

                    break;

                }

            }

        }

        channel_send(1, target, EVENT_END);
        channel_wait(1, target, EVENT_DONE);

    }

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

    unsigned int wm = lookup(option_getstring("wm-service"));

    channel_send(0, wm, EVENT_WMMAP);

    while (channel_process(0));

    channel_send(0, wm, EVENT_WMUNMAP);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char urldata[4096];
    struct url url;
    unsigned char buffer[4096];
    unsigned int count;
    struct event_clockinfo clockinfo;
    struct mtwist_state state;

    option_setdecimal("clock-service", lookup(option_getstring("clock-service")));
    option_setdecimal("ethernet-service", lookup(option_getstring("ethernet-service")));
    channel_send(0, option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(0, option_getdecimal("clock-service"), EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
    mtwist_seed1(&state, time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(0, option_getdecimal("ethernet-service"), &local);
    parseurl(&url, urldata, 4096);

    if (url.host)
        dnsresolve(&remote, url.host);

    if (url.port)
        socket_bind_tcps(&remote, url.port, mtwist_rand(&state), mtwist_rand(&state));

    channel_send(0, option_getdecimal("ethernet-service"), EVENT_LINK);
    socket_resolveremote(0, option_getdecimal("ethernet-service"), &local, &router);
    socket_connect_tcp(0, option_getdecimal("ethernet-service"), &local, &remote, &router);
    socket_send_tcp(0, option_getdecimal("ethernet-service"), &local, &remote, &router, buildrequest(4096, buffer, &url), buffer);

    while ((count = socket_receive(0, option_getdecimal("ethernet-service"), &local, &remote, 1, &router, buffer, 4096)))
    {

        if (ring_write(&input, buffer, count))
            handlehttppacket(source);

    }

    channel_send(0, option_getdecimal("ethernet-service"), EVENT_UNLINK);

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
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "10.0.5.80");
    option_add("url", "");
    option_add("dns", "initrd:bin/dns");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

}

