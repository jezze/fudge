#include <fudge.h>
#include <abi.h>

static unsigned int isaddress(char *host)
{

    unsigned int dots = 0;

    for (; *host; host++)
    {

        if (*host == '.')
            dots++;
        else if (*host < '0' || *host > '9')
            return 0;

    }

    return dots == 3;

}

static void opensocket(unsigned int source, struct url *url, char address[32])
{

    unsigned int target = fs_spawn(2, 2, "initrd:bin/socket");

    if (target)
    {

        struct message message;

        channel_send_fmt(2, target, EVENT_OPTION, "mode\\0tcp\\0remote-address\\0%s\\0remote-port\\0%s\\0", address, (url->port) ? url->port : "80");
        channel_send(2, target, EVENT_MAIN, 0, 0);

        if (channel_poll(2, target, EVENT_READY, &message))
        {

            channel_send_fmt(2, target, EVENT_QUERYREQUEST, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", (url->path) ? url->path : "", url->host);

            while (channel_poll(2, target, EVENT_DATA, &message))
                channel_send(0, source, EVENT_DATA, message.length, message.data);

        }

    }

}

static void onmain(struct message *message)
{

    char *opturl = option_getstring("url");
    unsigned int count = cstring_length(opturl);

    if (count)
    {

        char urldata[2048];
        char address[32];
        struct url url;

        if (cstring_length(opturl) >= 4 && buffer_match(opturl, "http", 4))
            url_parse(&url, urldata, 2048, opturl, URL_SCHEME);
        else
            url_parse(&url, urldata, 2048, opturl, URL_HOST);

        if (isaddress(url.host))
            opensocket(message->source, &url, url.host);
        else if (system_resolve(url.host, address, 32))
            opensocket(message->source, &url, address);
        else
            channel_send_fmt(0, message->source, EVENT_ERROR, "Could not resolve: %s\n", url.host);

    }

}

void init(void)
{

    option_add("url", "");
    channel_bind(EVENT_MAIN, onmain);

}

