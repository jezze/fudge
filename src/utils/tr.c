#include <fudge.h>
#include <abi.h>

static char from;
static char to;

static char parsechar(char *s)
{

    if (s[0] != '\\')
        return s[0];

    switch (s[1])
    {

    case 'n':
        return '\n';

    case 't':
        return '\t';

    case '0':
        return '\0';

    default:
        return s[1];

    }

}

static void replace(unsigned int source, struct message *message)
{

    char *buffer = message->data;
    unsigned int i;

    for (i = 0; i < message->length; i++)
    {

        if (buffer[i] == from)
            buffer[i] = to;

    }

    channel_forward(0, source, MESSAGE_TYPE_DATA, message);

}

/* in place, so data that came in a page goes on in that page */
static void ondata(struct message *message)
{

    replace(message->source, message);

}

static void onpath(struct message *message)
{

    fs_read_each(1, message->source, message->data, replace);

}

static void onmain(struct message *message)
{

    from = parsechar(option_getstring("f"));
    to = parsechar(option_getstring("t"));

}

void init(void)
{

    option_add("f", "");
    option_add("t", "");
    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_PATH, onpath);

}

