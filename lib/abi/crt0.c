#include <fudge.h>
#include <abi.h>

extern void init(void);

static unsigned int started;

static void onoption(struct message *message)
{

    struct message_option *option = message_validate(MESSAGE_TYPE_OPTION, message->data, message->length);

    if (!option)
        return;

    if (!option_getstring(option->key))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Unrecognized option: %s\n", option->key);
    else if (!option_setstring(option->key, option->value))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Option too long: %s\n", option->key);

}

void panic(unsigned int source, char *file, unsigned int line)
{

    channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Process panic! File %s on line %u\n", file, &line);
    channel_exit(0);
    call_despawn();

}

void main(void)
{

    if (started)
    {

        channel_abort(0);
        call_despawn();

    }

    started = 1;

    option_add("pwd", "");
    channel_bind(MESSAGE_TYPE_OPTION, onoption);
    init();
    channel_loop(0);
    channel_exit(0);
    call_despawn();

}

