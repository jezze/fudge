#include <fudge.h>
#include <abi.h>

extern void init(void);

static unsigned int eachentry(char *data, unsigned int length, unsigned int offset, unsigned int xcount, char *x)
{

    unsigned int i;

    for (i = offset; i < length; i++)
    {

        unsigned int j;

        for (j = 0; j < xcount; j++)
        {

            if (data[i] == x[j])
                return i + 1 - offset;

        }

    }

    return 0;

}

static char *extract(char *data, unsigned int length, unsigned int offset)
{

    data[offset + length - 1] = '\0';

    return data + offset;

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int offset;
    unsigned int klength;
    unsigned int vlength;

    for (offset = 0; (klength = eachentry(mdata, msize, offset, 1, "=")) && (vlength = eachentry(mdata, msize, offset + klength, 3, "&\n\0")); offset += klength + vlength)
    {

        char *key = extract(mdata, klength, offset);
        char *value = extract(mdata, vlength, offset + klength);

        if (!option_set(key, value))
            channel_send_fmt1(0, source, EVENT_ERROR, "Unrecognized option: %s\n", key);

    }

}

static void onstatus(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int i;

    for (i = 0; i < OPTION_MAX; i++)
    {

        struct option *option = option_get(i);

        if (option)
            channel_send_fmt2(0, source, EVENT_DATA, "%s:%s\n", option->key, option->value);

    }

}

void lookup(char *key)
{

    char id[32];

    channel_send_fmt1(0, option_getdecimal("env"), EVENT_QUERYREQUEST, "get\\0%s\\0", key);
    channel_wait_buffer(0, option_getdecimal("env"), EVENT_QUERYRESPONSE, 32, id);
    option_set(key, id);

}

void lookup2(char *key, char *name, unsigned int index, unsigned int inode)
{

    option_setdecimal(key, call_find(cstring_length(name), name, index, inode));

}

void panic(unsigned int source, char *file, unsigned int line)
{

    channel_send_fmt2(0, source, EVENT_ERROR, "Process panic! File %s on line %u\n", file, &line);
    channel_close();
    call_despawn();

}

void main(void)
{

    option_add("env", "");
    option_add("pwd", "");
    channel_open();
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_STATUS, onstatus);
    init();

    while (channel_process(0));

}

