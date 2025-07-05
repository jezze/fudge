#include <fudge.h>
#include <abi.h>

static char strings[8192];
static unsigned int stringslength;

static unsigned int eachentry(char *data, unsigned int length, unsigned int offset)
{

    unsigned int i;

    for (i = offset; i < length; i++)
    {

        if (data[i] == '\0')
            return i + 1 - offset;

    }

    return 0;

}

static unsigned int matchkey(char *data, unsigned int length, char *key, unsigned int keylength)
{

    return (length == keylength && buffer_match(data, key, keylength));

}

static char *find(char *key, unsigned int keylength)
{

    unsigned int offset;
    unsigned int length;

    for (offset = 0; (length = eachentry(strings, stringslength, offset)); offset += length)
    {

        if (matchkey(strings + offset, length, key, keylength))
            return strings + offset + length;

    }

    return 0;

}

static void onqueryrequest(unsigned int source, void *mdata, unsigned int msize)
{

    char *cmd = mdata;
    unsigned int cmdlength = eachentry(mdata, msize, 0);

    if (matchkey(cmd, cmdlength, "get", 4))
    {

        char *key = cmd + cmdlength;
        unsigned int keylength = eachentry(mdata, msize, cmdlength);
        char *value = find(key, keylength);

        if (value)
            channel_send_fmt1(0, source, EVENT_QUERYRESPONSE, "%s\\0", value);

    }

    else if (matchkey(cmd, cmdlength, "set", 4))
    {

        char *key = cmd + cmdlength;
        unsigned int keylength = eachentry(mdata, msize, cmdlength);
        char *value = key + keylength;
        unsigned int valuelength = eachentry(mdata, msize, cmdlength + keylength);

        if (keylength && valuelength)
        {

            stringslength += cstring_write_fmt1(strings, 8192, stringslength, "%s\\0", key);

            if (matchkey(value, valuelength, "!source", 8))
                stringslength += cstring_write_fmt1(strings, 8192, stringslength, "%u\\0", &source);
            else
                stringslength += cstring_write_fmt1(strings, 8192, stringslength, "%s\\0", value);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_QUERYREQUEST, onqueryrequest);

}

