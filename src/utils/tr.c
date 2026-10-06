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

static void replace(unsigned int source, char *buffer, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (buffer[i] == from)
            buffer[i] = to;

    }

    channel_send(0, source, EVENT_DATA, count, buffer);

}

static void ondata(struct message *message)
{

    char buffer[MESSAGE_SIZE];

    replace(message->source, buffer, buffer_write(buffer, MESSAGE_SIZE, message->data, message->length, 0));

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
        {

            char buffer[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, MESSAGE_SIZE, offset)); offset += count)
                replace(message->source, buffer, count);

        }

        else
        {

            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

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
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

