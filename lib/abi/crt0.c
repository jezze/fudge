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

static void oninterrupt(struct message *message)
{

    channel_route(EVENT_DONE, message->source);

}

static void onoption(struct message *message)
{

    unsigned int offset;
    unsigned int klength;
    unsigned int vlength;

    for (offset = 0; (klength = eachentry(message->data, message->length, offset, 1, "=")) && (vlength = eachentry(message->data, message->length, offset + klength, 3, "&\n\0")); offset += klength + vlength)
    {

        char *key = extract(message->data, klength, offset);
        char *value = extract(message->data, vlength, offset + klength);

        if (!option_setstring(key, value))
            channel_send_fmt1(0, message->source, EVENT_ERROR, "Unrecognized option: %s\n", key);

    }

}

static void onterm(struct message *message)
{

    channel_route(EVENT_DONE, message->source);

}

void panic(unsigned int source, char *file, unsigned int line)
{

    channel_send_fmt2(0, source, EVENT_ERROR, "Process panic! File %s on line %u\n", file, &line);
    call_despawn();

}

void main(void)
{

    option_add("pwd", "");
    channel_bind(EVENT_INTERRUPT, oninterrupt);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_TERM, onterm);
    init();

}

