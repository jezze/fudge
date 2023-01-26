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
            channel_sendbuffer(EVENT_WMRENDERDATA, count, buffer);

        }

        else
        {

            if (count == 2 && buffer[0] == '\r' && buffer[1] == '\n')
                isbody = 1;

        }

    }

}

static unsigned int buildrequest(unsigned int count, void *buffer, struct url *url)
{

    unsigned int offset = 0;

    offset += cstring_write(buffer, count, "GET /", offset);

    if (url->path)
        offset += cstring_write(buffer, count, url->path, offset);

    offset += cstring_write(buffer, count, " HTTP/1.1\r\n", offset);
    offset += cstring_write(buffer, count, "Host: ", offset);
    offset += cstring_write(buffer, count, url->host, offset);
    offset += cstring_write(buffer, count, "\r\n\r\n", offset);

    return offset;

}

static void dnsresolve(char *domain)
{

    unsigned int id = file_spawn(FILE_L0, "/bin/dns");

    if (id)
    {

        struct message_header header;
        char data[MESSAGE_SIZE];

        channel_redirectback(id, EVENT_QUERY);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendfmt1to(id, EVENT_OPTION, "domain\\0%s\\0", domain);
        channel_sendto(id, EVENT_MAIN);

        while (channel_pollfrom(id, &header, MESSAGE_SIZE, data) != EVENT_CLOSE)
        {

            if (header.event == EVENT_QUERY)
            {

                char *key = cstring_tindex(data, message_datasize(&header), 0);
                char *value = cstring_tindex(data, message_datasize(&header), 1);

                if (cstring_match(key, "data"))
                    option_set("remote-address", value);

            }

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

    if (file_walk2(FILE_L0, "system:service/wm"))
        file_notify(FILE_L0, EVENT_WMMAP, 0, 0);
    else
        channel_warning("Could not open window manager service");

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(EVENT_WMUNMAP);
    channel_close();

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char urldata[BUFFER_SIZE];
    struct url url;
    unsigned char buffer[BUFFER_SIZE];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);
    parseurl(&url, urldata, BUFFER_SIZE);

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

    if (url.host)
        dnsresolve(url.host);

    socket_bind_ipv4s(&remote, option_getstring("remote-address"));

    if (url.port)
        socket_bind_tcps(&remote, url.port, mtwist_rand(&state), mtwist_rand(&state));
    else
        socket_bind_tcpv(&remote, option_getdecimal("remote-port"), mtwist_rand(&state), mtwist_rand(&state));

    count = buildrequest(BUFFER_SIZE, buffer, &url);

    file_link(FILE_G0);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_connect_tcp(FILE_G0, &local, &remote, &router);
    socket_send_tcp(FILE_G0, &local, &remote, &router, count, buffer);

    while ((count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, BUFFER_SIZE)))
    {

        if (ring_write(&input, buffer, count))
            handlehttppacket();

    }

    file_unlink(FILE_G0);

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
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMINIT, onwminit);

}

