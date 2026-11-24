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

    unsigned int secure = url->scheme && cstring_match(url->scheme, "https");
    char *mode = (secure) ? "tls" : "tcp";
    char *port = (url->port) ? url->port : ((secure) ? "443" : "80");

    unsigned int target = fs_spawn(1, "initrd:bin/socket");

    if (target)
    {

        struct message_pipe pipe;
        struct message *message;

        channel_sendoption(1, target, "mode", mode);
        channel_sendoption(1, target, "remote-address", address);
        channel_sendoption(1, target, "remote-port", port);
        channel_sendoption(1, target, "server-name", url->host);
        message_initpipe(&pipe);
        channel_send(1, target, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);

        if ((message = channel_poll(1, target, MESSAGE_TYPE_READY)))
        {

            channel_send_fmt(1, target, MESSAGE_TYPE_QUERYREQUEST, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", (url->path) ? url->path : "", url->host);

            while ((message = channel_poll(1, target, MESSAGE_TYPE_DATA)))
                channel_forward(0, source, MESSAGE_TYPE_DATA, message);

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
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not resolve: %s\n", url.host);

    }

}

void init(void)
{

    option_add("url", "");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

