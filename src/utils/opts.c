#include <fudge.h>
#include <abi.h>

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);
    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, key, offset);
    offset = message_putstring(&data, " ", offset);
    offset = message_putstring(&data, value, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_reply(channel, EVENT_DATA, offset, &data);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_OPTION, onoption);

}

