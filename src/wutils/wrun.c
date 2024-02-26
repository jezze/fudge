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

    return cstring_write_fmt2(buffer, count, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", 0, (url->path) ? url->path : "", url->host);

}

static void handlehttppacket(void)
{

    unsigned int newline;

    while ((newline = ring_each(&input, '\n')))
    {

        char buffer[4096];
        unsigned int count = ring_read(&input, buffer, newline);

        if (isbody)
        {

            channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);
            channel_send_buffer(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, count, buffer);

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

    channel_send(option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process());

    channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char urldata[4096];
    struct url url;
    unsigned char buffer[4096];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);
    setupnetwork(&state);
    parseurl(&url, urldata, 4096);

    if (url.host)
        dnsresolve(&remote, url.host);

    if (url.port)
        socket_bind_tcps(&remote, url.port, mtwist_rand(&state), mtwist_rand(&state));

    call_link(FILE_G0, 8000);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_connect_tcp(FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, buildrequest(4096, buffer, &url), buffer);

    while ((count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, 4096)))
    {

        if (ring_write(&input, buffer, count))
            handlehttppacket();

    }

    call_unlink(FILE_G0);

}

void init(void)
{

    ring_init(&input, 4096, inputdata);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("wm-service", "12345");
    option_add("clock", "system:clock/if.0");
    option_add("ethernet", "system:ethernet/if.0");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "");
    option_add("remote-port", "80");
    option_add("router-address", "10.0.5.80");
    option_add("url", "");
    option_add("dns", "initrd:/bin/dns");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMINIT, onwminit);

}

