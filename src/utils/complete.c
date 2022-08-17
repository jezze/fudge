#include <fudge.h>
#include <abi.h>

static char keyname[1024];
static unsigned int keylength;

static void print(unsigned int source, struct record *record)
{

    if (buffer_match(record->name, keyname, keylength))
    {

        struct message message;

        message_init(&message, EVENT_DATA);
        message_putbuffer(&message, record->length, record->name);
        message_putstring(&message, "\n");
        channel_sendmessage(&message);

    }

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthz(key);

    if (cstring_match(key, "key"))
        keylength = cstring_write(keyname, 1024, value, 0);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        do
        {

            struct record record;

            if (file_readall(FILE_L0, &record, sizeof (struct record)))
                print(source, &record);

        } while (file_step(FILE_L0));

    }

    else
    {

        channel_error("File not found");

    }

}

void init(void)
{

    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_PATH, onpath);

}

