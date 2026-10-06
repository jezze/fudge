#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"
#include "option.h"
#include "system.h"

static unsigned int start(char *command, unsigned int input)
{

    unsigned int sh = fs_spawn(1, 1, "initrd:bin/sh");

    if (sh)
    {

        unsigned int length = cstring_length(command);
        unsigned int offset;

        channel_send_fmt2(1, sh, EVENT_OPTION, "pwd=%s&input=%u\n", option_getstring("pwd"), &input);
        channel_send(1, sh, EVENT_MAIN, 0, 0);

        for (offset = 0; offset < length; offset += MESSAGE_SIZE)
            channel_send(1, sh, EVENT_DATA, (length - offset < MESSAGE_SIZE) ? length - offset : MESSAGE_SIZE, command + offset);

        channel_send(1, sh, EVENT_DATA, 1, "\n");
        channel_send(1, sh, EVENT_TERM, 0, 0);

    }

    return sh;

}

unsigned int system_run(unsigned int target, char *command)
{

    if (start(command, 0))
    {

        struct message message;

        while (channel_pick(1, &message))
        {

            switch (message.event)
            {

            case EVENT_DATA:
            case EVENT_ERROR:
                channel_send(0, target, message.event, message.length, message.data);

                break;

            case EVENT_EXIT:
                return ((struct event_exit *)message.data)->status;

            }

        }

    }

    return 0;

}

unsigned int system_feed(char *command, void *input, unsigned int inputcount, void *output, unsigned int outputcount)
{

    unsigned int sh = start(command, 1);
    unsigned int total = 0;

    if (sh)
    {

        char *data = input;
        struct message message;
        unsigned int offset;

        for (offset = 0; offset < inputcount; offset += MESSAGE_SIZE)
            channel_send(1, sh, EVENT_DATA, (inputcount - offset < MESSAGE_SIZE) ? inputcount - offset : MESSAGE_SIZE, data + offset);

        channel_send(1, sh, EVENT_TERM, 0, 0);

        while (channel_pick(1, &message))
        {

            switch (message.event)
            {

            case EVENT_DATA:
                total += buffer_write(output, outputcount, message.data, message.length, total);

                break;

            case EVENT_EXIT:
                return total;

            }

        }

    }

    return total;

}
