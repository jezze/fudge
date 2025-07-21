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

void lookup(char *key)
{

    char id[32];

    channel_send_fmt1(0, option_getdecimal("env"), EVENT_QUERYREQUEST, "get\\0%s\\0", key);
    channel_wait_buffer(0, option_getdecimal("env"), EVENT_QUERYRESPONSE, 32, id);
    option_set(key, id);

}

unsigned int lookup2(char *name)
{

    unsigned int length = cstring_length(name);
    unsigned int offset = buffer_eachbyte(name, length, ':', 0);

    if (offset > 0)
    {

        unsigned int offindex = buffer_eachbyte(name, length, ':', offset);

        if (offindex > 0)
        {

            unsigned int index = name[offset] - '0';
            unsigned int inode = name[offindex] - '0';

            return call_find(offset - 1, name, index, inode);

        }

        else
        {

            unsigned int index = name[offset] - '0';

            return call_find(offset - 1, name, index, 0);

        }

    }

    else
    {

        return call_find(length, name, 0, 0);

    }

    return 0;

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
    init();

    while (channel_process(0));

}

