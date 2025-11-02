#include <fudge.h>
#include <abi.h>

static void dnsresolve(unsigned int source, char *domain, char address[32])
{

    unsigned int target = fs_spawn(1, option_getstring("dns"));

    if (target)
    {

        struct message message;
        char data[MESSAGE_SIZE];

        channel_send_fmt1(1, target, EVENT_OPTION, "domain=%s\n", domain);
        channel_send(1, target, EVENT_MAIN);

        if (channel_poll(1, target, EVENT_QUERYRESPONSE, &message, MESSAGE_SIZE, data))
        {

            unsigned int i;
            char *key;

            for (i = 0; (key = buffer_tindex(data, message.length, '\0', i)); i += 2)
            {

                if (cstring_match(key, "data"))
                {

                    char *value = key + cstring_length_zero(key);

                    buffer_write(address, 32, value, cstring_length_zero(value), 0);

                    break;

                }

            }

        }

        channel_send(1, target, EVENT_END);
        channel_wait(1, target, EVENT_DONE);

    }

}

static void opensocket(unsigned int source, struct url *url, char address[32])
{

    unsigned int target = fs_spawn(2, option_getstring("socket"));

    if (target)
    {

        struct message message;
        char data[MESSAGE_SIZE];

        channel_send_fmt1(2, target, EVENT_OPTION, "mode=tcp&remote-address=%s\n", address);
        channel_send(2, target, EVENT_MAIN);
        channel_wait(2, target, EVENT_READY);
        channel_send_fmt2(2, target, EVENT_QUERYREQUEST, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", (url->path) ? url->path : "", url->host);

        while (channel_poll(2, target, EVENT_DATA, &message, MESSAGE_SIZE, data))
            channel_send_buffer(2, source, EVENT_DATA, message.length, data);

        channel_send(2, target, EVENT_END);
        channel_wait(2, target, EVENT_DONE);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
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

        dnsresolve(source, url.host, address);
        opensocket(source, &url, address);

    }

}

void init(void)
{

    option_add("dns", "initrd:bin/dns");
    option_add("socket", "initrd:bin/socket");
    option_add("url", "");
    channel_bind(EVENT_MAIN, onmain);

}

