#include <fudge.h>
#include <abi.h>

static char key[1024];
static unsigned int keylength;

static void print(unsigned int source, struct record *record)
{

    if (buffer_match(record->name, key, keylength))
    {

        struct message message;

        message_init(&message, EVENT_DATA);
        message_putbuffer2(&message, record->length, record->name);
        message_putstring2(&message, "\n");
        channel_replymsg(&message);

    }

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "key"))
        keylength = buffer_write(key, 1024, value, ascii_length(value), 0);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        file_reset(FILE_L0);

        do
        {

            struct record record;

            if (file_readall(FILE_L0, &record, sizeof (struct record)))
                print(source, &record);

        } while (file_step(FILE_L0));

    }

}

void init(void)
{

    channel_setcallback(EVENT_OPTION, onoption);
    channel_setcallback(EVENT_PATH, onpath);

}

