#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define URLSIZE                         256
#define RESPONSESIZE                    0x8000

static char url[URLSIZE] = "http://";
static char response[RESPONSESIZE];

static void render(unsigned int wm, char *data, unsigned int count)
{

    unsigned int offset = 0;

    while (offset < count)
    {

        unsigned int length = count - offset;

        if (length > MESSAGE_SIZE)
        {

            length = buffer_lastbyte(data + offset, MESSAGE_SIZE, '\n');

            if (!length)
                length = MESSAGE_SIZE;

        }

        channel_send(0, wm, EVENT_WMRENDERDATA, length, data + offset);

        offset += length;

    }

}

static unsigned int isvalid(char *s, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (s[i] == '"')
            return 0;

    }

    return 1;

}

static void seturl(char *s)
{

    cstring_write_fmt(url, URLSIZE, 0, "%s\\0", s);

    url[URLSIZE - 1] = '\0';

}

static void showstatus(unsigned int wm, char *text)
{

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= status label \"%s\"\n", text);

}

static void open(unsigned int wm)
{

    char urldata[URLSIZE * 2];
    struct url parsed;
    unsigned int count;
    unsigned int i;

    if (!isvalid(url, cstring_length(url)))
    {

        showstatus(wm, "The address can not contain quotes");

        return;

    }

    url_parse(&parsed, urldata, URLSIZE * 2, url, (buffer_match(url, "http", 4)) ? URL_SCHEME : URL_HOST);

    if (!parsed.host || !cstring_length(parsed.host))
    {

        showstatus(wm, "Enter an address to open");

        return;

    }

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= status label \"Connecting to %s...\"\n", parsed.host);

    count = system_feed(0, 0, response, RESPONSESIZE, "webc -url \"%s\"", url);

    for (i = 0; i + 4 <= count; i++)
    {

        if (buffer_match(response + i, "\r\n\r\n", 4))
        {

            unsigned int length = buffer_findbyte(response, count, '\r');

            if (count > 12 && buffer_match(response + 8, " 200", 4))
            {

                channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "- window\n");
                render(wm, response + i + 4, count - i - 4);

            }

            else if (isvalid(response, length))
            {

                channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= status label \"%w\"\n", response, &length);

            }

            else
            {

                showstatus(wm, "The server answered with an error");

            }

            return;

        }

    }

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= status label \"Could not load from %s\"\n", parsed.host);

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

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;
    char *data = (char *)(event + 1);

    /* the address is taken as is instead of through kv, since a URL can contain & */
    if (event->length > 14 && buffer_match(data, "q=address&url=", 14))
        seturl(data + 14);
    else if (kv_match(event, "q=open"))
        open(message->source);

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wrun.alfi";

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);

    if (cstring_length(option_getstring("url")))
    {

        seturl(option_getstring("url"));
        channel_send_fmt(0, message->source, EVENT_WMRENDERDATA, "= address label \"%s\"\n", url);
        open(message->source);

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    option_add("url", "");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

