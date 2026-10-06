#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"
#include "option.h"
#include "system.h"

static unsigned int start(char *command)
{

    unsigned int sh = fs_spawn(1, 1, "initrd:bin/sh");

    if (sh)
    {

        unsigned int length = cstring_length(command);
        unsigned int offset;

        channel_send_fmt1(1, sh, EVENT_OPTION, "pwd=%s\n", option_getstring("pwd"));
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

    if (start(command))
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

unsigned int system_read(char *command, void *buffer, unsigned int count)
{

    unsigned int total = 0;

    if (start(command))
    {

        struct message message;

        while (channel_pick(1, &message))
        {

            switch (message.event)
            {

            case EVENT_DATA:
                total += buffer_write(buffer, count, message.data, message.length, total);

                break;

            case EVENT_EXIT:
                return total;

            }

        }

    }

    return total;

}

