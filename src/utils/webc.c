#include <fudge.h>
#include <abi.h>

static void opensocket(struct url *url)
{

    unsigned int channel = call_spawn_absolute(FILE_L0, FILE_PW, option_getstring("socket"));

    if (channel)
    {

        char data[MESSAGE_SIZE];
        unsigned int count = cstring_write_fmt2(data, MESSAGE_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", 0, (url->path) ? url->path : "", url->host);

        channel_listen(channel, EVENT_DATA);
        channel_listen(channel, EVENT_CLOSE);
        channel_send_fmt1(channel, EVENT_OPTION, "remote-address\\0%s\\0", option_getstring("host"));
        channel_send(channel, EVENT_MAIN);
        channel_send_fmt2(channel, EVENT_DATA, "%w", data, &count);

        while ((count = channel_read_from(channel, EVENT_DATA, data)))
            channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, data);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", option_getstring("socket"));

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

    char urldata[2048];
    struct url url;

    parseurl(&url, urldata, 2048);
    opensocket(&url);

}

void init(void)
{

    option_add("socket", "initrd:/bin/socket");
    option_add("host", "51.158.105.96");
    option_add("url", "www.blunder.se");
    channel_bind(EVENT_MAIN, onmain);

}

