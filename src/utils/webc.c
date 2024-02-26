#include <fudge.h>
#include <abi.h>

static void dnsresolve(char *domain, char address[32])
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

                    buffer_write(address, 32, value, cstring_length_zero(value), 0);

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

static void opensocket(struct url *url, char address[32])
{

    unsigned int channel = call_spawn_absolute(FILE_L0, FILE_PW, option_getstring("socket"));

    if (channel)
    {

        char data[MESSAGE_SIZE];
        unsigned int count = cstring_write_fmt2(data, MESSAGE_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", 0, (url->path) ? url->path : "", url->host);

        channel_listen(channel, EVENT_DATA);
        channel_listen(channel, EVENT_CLOSE);
        channel_send_fmt1(channel, EVENT_OPTION, "mode\\0tcp\\0remote-address\\0%s\\0", address);
        channel_send(channel, EVENT_MAIN);
        channel_send_fmt2(channel, EVENT_DATA, "%w", data, &count);

        while ((count = channel_read_from(channel, EVENT_DATA, data)))
            channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, data);

        channel_send(channel, EVENT_TERM);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", option_getstring("socket"));

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

        dnsresolve(url.host, address);
        opensocket(&url, address);

    }

}

void init(void)
{

    option_add("dns", "initrd:/bin/dns");
    option_add("socket", "initrd:/bin/socket");
    option_add("url", "");
    channel_bind(EVENT_MAIN, onmain);

}

