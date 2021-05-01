#include <fudge.h>
#include <abi.h>

static char key[1024];
static unsigned int keylength;

static void print(struct channel *channel, unsigned int source, struct record *record)
{

    if (buffer_match(record->name, key, keylength))
    {

        struct message_data data;
        unsigned int offset = 0;

        offset = message_putbuffer(&data, record->length, record->name, offset);
        offset = message_putstring(&data, "\n", offset);

        channel_reply(channel, EVENT_DATA, offset, &data);

    }

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "key"))
        keylength = buffer_write(key, 1024, value, ascii_length(value), 0);

}

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        file_open(FILE_L0);

        do
        {

            struct record record;

            if (file_readall(FILE_L0, &record, sizeof (struct record)))
                print(channel, source, &record);

        } while (file_step(FILE_L0));

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_OPTION, onoption);
    channel_setcallback(channel, EVENT_PATH, onpath);

}

