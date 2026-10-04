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

static void onoption(struct message *message)
{

    unsigned int offset;
    unsigned int klength;
    unsigned int vlength;

    for (offset = 0; (klength = eachentry(message->data, message->length, offset, 1, "=")) && (vlength = eachentry(message->data, message->length, offset + klength, 3, "&\n\0")); offset += klength + vlength)
    {

        char *key = extract(message->data, klength, offset);
        char *value = extract(message->data, vlength, offset + klength);

        if (!option_getstring(key))
            channel_send_fmt1(0, message->source, EVENT_ERROR, "Unrecognized option: %s\n", key);
        else if (!option_setstring(key, value))
            channel_send_fmt1(0, message->source, EVENT_ERROR, "Option too long: %s\n", key);

    }

}

static void onpipe(struct message *message)
{

    struct event_pipe *pipe = message->data;

    channel_pipe(message->source, pipe->prev, (pipe->next) ? pipe->next : message->source);

}

void panic(unsigned int source, char *file, unsigned int line)
{

    channel_send_fmt2(0, source, EVENT_ERROR, "Process panic! File %s on line %u\n", file, &line);
    channel_close(0);
    call_despawn();

}

void main(void)
{

    option_add("pwd", "");
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_PIPE, onpipe);
    init();
    channel_loop(0);
    channel_close(0);

}

