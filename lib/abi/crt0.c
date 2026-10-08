#include <fudge.h>
#include <abi.h>

extern void init(void);

static void onoption(struct message *message)
{

    char *data = message->data;
    unsigned int offset = 0;

    if (!message->length || data[message->length - 1] != '\0')
        return;

    while (offset < message->length)
    {

        char *key = data + offset;
        char *value = key + cstring_length_zero(key);

        offset += cstring_length_zero(key);

        if (offset >= message->length)
            break;

        offset += cstring_length_zero(value);

        if (!option_getstring(key))
            channel_send_fmt(0, message->source, EVENT_ERROR, "Unrecognized option: %s\n", key);
        else if (!option_setstring(key, value))
            channel_send_fmt(0, message->source, EVENT_ERROR, "Option too long: %s\n", key);

    }

}

static void onpipe(struct message *message)
{

    channel_pipe(message->source, message->data);

}

void panic(unsigned int source, char *file, unsigned int line)
{

    channel_send_fmt(0, source, EVENT_ERROR, "Process panic! File %s on line %u\n", file, &line);
    channel_exit(0);
    call_despawn(EXIT_STATUS_FAILED);

}

void main(void)
{

    option_add("pwd", "");
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_PIPE, onpipe);
    init();
    channel_loop(0);
    channel_exit(0);
    call_despawn(EXIT_STATUS_NORMAL);

}

