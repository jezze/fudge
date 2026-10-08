#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"
#include "option.h"
#include "system.h"

static unsigned int start(char *command, unsigned int length, unsigned int input)
{

    unsigned int sh = fs_spawn(1, 1, "initrd:bin/sh");

    if (sh)
    {

        unsigned int offset;

        channel_send_fmt(1, sh, EVENT_OPTION, "pwd\\0%s\\0input\\0%u\\0", option_getstring("pwd"), &input);
        channel_send(1, sh, EVENT_MAIN, 0, 0);

        for (offset = 0; offset < length; offset += MESSAGE_SIZE)
            channel_send(1, sh, EVENT_DATA, (length - offset < MESSAGE_SIZE) ? length - offset : MESSAGE_SIZE, command + offset);

        channel_send(1, sh, EVENT_DATA, 1, "\n");
        channel_send(1, sh, EVENT_TERM, 0, 0);

    }

    return sh;

}

unsigned int system_runv(unsigned int target, char *fmt, void **args)
{

    char command[MESSAGE_SIZE];
    unsigned int errors = 0;

    if (start(command, cstring_write_fmtv(command, MESSAGE_SIZE, 0, fmt, args), 0))
    {

        struct message message;

        while (channel_pick(1, &message))
        {

            switch (message.event)
            {

            case EVENT_ERROR:
                errors++;

            case EVENT_DATA:
                channel_send(0, target, message.event, message.length, message.data);

                break;

            case EVENT_EXIT:
                return (errors) ? EXIT_STATUS_FAILED : ((struct event_exit *)message.data)->status;

            }

        }

    }

    return 0;

}

unsigned int system_feedv(void *input, unsigned int inputcount, void *output, unsigned int outputcount, char *fmt, void **args)
{

    char command[MESSAGE_SIZE];
    unsigned int sh = start(command, cstring_write_fmtv(command, MESSAGE_SIZE, 0, fmt, args), 1);
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

char *system_address(char *option, char *key, char *out, unsigned int size)
{

    char *value = option_getstring(option);
    unsigned int count;

    if (value && cstring_length(value))
        return value;

    count = system_feed(0, 0, out, size - 1, "mq -query .networks[0].%s initrd:data/config/network.mq", key);

    while (count && out[count - 1] == '\n')
        count--;

    out[count] = '\0';

    return out;

}

unsigned int system_unixtime(char *service)
{

    unsigned int clock = channel_lookup(service);
    struct event_clockinfo clockinfo;

    if (!clock)
        return 0;

    channel_send(1, clock, EVENT_INFO, 0, 0);

    if (!channel_wait(1, clock, EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo))
        return 0;

    return time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds);

}

unsigned int system_resolve(char *domain, char *address, unsigned int size)
{

    char answers[MESSAGE_SIZE];
    unsigned int count;
    unsigned int length = 0;
    unsigned int i;
    char *key;

    count = system_feed(0, 0, answers, MESSAGE_SIZE, "dns -domain %s queryresponse>&data", domain);

    for (i = 0; (key = buffer_tindex(answers, count, '\0', i)); i += 2)
    {

        if (cstring_match(key, "data"))
        {

            char *value = key + cstring_length_zero(key);

            length = buffer_write(address, size, value, cstring_length_zero(value), 0);

        }

    }

    return length;

}
