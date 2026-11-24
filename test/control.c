#include <fudge.h>
#include <abi.h>

/* control messages have a fixed size: one too long is not sent, and the sender's owner gets an error instead */
static void onmain(struct message *message)
{

    unsigned int helper = fs_spawn(1, "initrd:test/longmsg");
    unsigned int errors = 0;
    unsigned int sent = 0;
    struct message *current;
    struct message_pipe pipe;

    if (!helper)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "spawn failed\n");

        return;

    }

    message_initpipe(&pipe);
    channel_send(1, helper, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
    channel_send(1, helper, MESSAGE_TYPE_TERM, 0, 0);

    while ((current = channel_pick(1)))
    {

        if (current->source != helper)
            continue;

        if (current->type == MESSAGE_TYPE_ERROR && current->length >= 24 && buffer_match(current->data, "Control message too long", 24))
            errors++;

        if (current->type == MESSAGE_TYPE_OPTION || current->type == MESSAGE_TYPE_PATH)
            sent++;

        if (current->type == MESSAGE_TYPE_STATUS)
            break;

    }

    if (errors != 2 || sent)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%u errors, %u too long messages sent\n", &errors, &sent);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

