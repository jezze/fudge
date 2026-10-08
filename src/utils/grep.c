#include <fudge.h>
#include <abi.h>

static char line[MESSAGE_SIZE];
static unsigned int linecount;

static void checkline(unsigned int source)
{

    char *prefix = option_getstring("prefix");
    char *substr = option_getstring("substr");
    unsigned int prefixcount = cstring_length(prefix);
    unsigned int substrcount = cstring_length(substr);
    unsigned int match = prefixcount && prefixcount <= linecount && buffer_match(line, prefix, prefixcount);
    unsigned int i;

    for (i = 0; !match && substrcount && i + substrcount <= linecount; i++)
        match = buffer_match(line + i, substr, substrcount);

    if (match)
        channel_send(0, source, EVENT_DATA, linecount, line);

    linecount = 0;

}

static void check(unsigned int source, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        line[linecount++] = b[i];

        if (b[i] == '\n' || b[i] == '\0' || linecount == MESSAGE_SIZE - 1)
            checkline(source);

    }

}

static void flush(unsigned int source)
{

    if (linecount)
    {

        line[linecount++] = '\n';

        checkline(source);

    }

}

static void ondata(struct message *message)
{

    check(message->source, message->data, message->length);

}

static void onpath(struct message *message)
{

    if (fs_read_each(1, message->source, message->data, check))
        flush(message->source);

}

static void onterm(struct message *message)
{

    flush(message->source);

}

void init(void)
{

    option_add("prefix", "");
    option_add("substr", "");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

