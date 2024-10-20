#include <fudge.h>
#include <abi.h>

static void dnsresolve(unsigned int source, char *domain, char address[32])
{

    unsigned int channel = fs_spawn(0, option_getstring("dns"));

    if (channel)
    {

        struct message message;
        char data[MESSAGE_SIZE];

        channel_send_fmt1(0, channel, EVENT_OPTION, "domain\\0%s\\0", domain);
        channel_send(0, channel, EVENT_MAIN);

        if (channel_poll(0, channel, EVENT_QUERYRESPONSE, &message, MESSAGE_SIZE, data))
        {

            unsigned int i;
            char *key;

            for (i = 0; (key = buffer_tindex(data, message_datasize(&message), '\0', i)); i += 2)
            {

                if (cstring_match(key, "data"))
                {

                    char *value = key + cstring_length_zero(key);

                    buffer_write(address, 32, value, cstring_length_zero(value), 0);

                    break;

                }

            }

        }

        channel_send(0, channel, EVENT_END);
        channel_wait(0, channel, EVENT_DONE);

    }

}

static void opensocket(unsigned int source, struct url *url, char address[32])
{

    unsigned int channel = fs_spawn(0, option_getstring("socket"));

    if (channel)
    {

        struct message message;
        char data[MESSAGE_SIZE];

        channel_send_fmt1(0, channel, EVENT_OPTION, "mode\\0tcp\\0remote-address\\0%s\\0", address);
        channel_send(0, channel, EVENT_MAIN);
        channel_wait(0, channel, EVENT_READY);
        channel_send_fmt2(0, channel, EVENT_QUERYREQUEST, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", (url->path) ? url->path : "", url->host);

        while (channel_poll(0, channel, EVENT_DATA, &message, MESSAGE_SIZE, data))
            channel_send_buffer(0, source, EVENT_DATA, message_datasize(&message), data);

        channel_send(0, channel, EVENT_END);
        channel_wait(0, channel, EVENT_DONE);

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

