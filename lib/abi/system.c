#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"
#include "option.h"
#include "system.h"

static unsigned int start(char *command, unsigned int length, unsigned int input)
{

    unsigned int sh = fs_spawn(1, "initrd:bin/sh");

    if (sh)
    {

        struct message_pipe pipe;
        unsigned int offset;

        channel_send_fmt(1, sh, MESSAGE_TYPE_OPTION, "pwd\\0%s\\0input\\0%u\\0", option_getstring("pwd"), &input);
        message_initpipe(&pipe);
        channel_send(1, sh, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);

        for (offset = 0; offset < length; offset += MESSAGE_SIZE)
            channel_send(1, sh, MESSAGE_TYPE_DATA, (length - offset < MESSAGE_SIZE) ? length - offset : MESSAGE_SIZE, command + offset);

        channel_send(1, sh, MESSAGE_TYPE_DATA, 1, "\n");
        channel_send(1, sh, MESSAGE_TYPE_TERM, 0, 0);

    }

    return sh;

}

unsigned int system_runv(unsigned int target, char *fmt, void **args)
{

    char command[MESSAGE_SIZE];

    if (!target)
        target = channel_lookup("log");

    if (start(command, cstring_write_fmtv(command, MESSAGE_SIZE, 0, fmt, args), 0))
    {

        struct message message;
        unsigned int errors = 0;

        while (channel_pick(1, &message))
        {

            switch (message.type)
            {

            case MESSAGE_TYPE_ERROR:
                errors++;

            case MESSAGE_TYPE_DATA:
                channel_send(0, target, message.type, message.length, message.data);

                break;

            case MESSAGE_TYPE_TERM:
                return !errors;

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
            channel_send(1, sh, MESSAGE_TYPE_DATA, (inputcount - offset < MESSAGE_SIZE) ? inputcount - offset : MESSAGE_SIZE, data + offset);

        channel_send(1, sh, MESSAGE_TYPE_TERM, 0, 0);

        while (channel_pick(1, &message))
        {

            switch (message.type)
            {

            case MESSAGE_TYPE_DATA:
                total += buffer_write(output, outputcount, message.data, message.length, total);

                break;

            case MESSAGE_TYPE_TERM:
                return total;

            }

        }

    }

    return total;

}

static unsigned int readentry(char *path, char *device, char *key, char *out, unsigned int size)
{

    static char data[0x1000];
    unsigned int count = system_feed(0, 0, data, 0x1000, "mq -query \".networks[device:%s]\" -format \"{type} {%s}\" %s", device, key, path);
    unsigned int length = buffer_findbyte(data, count, '\n');
    unsigned int typelength = buffer_findbyte(data, length, ' ');
    unsigned int valuelength;

    if (typelength == length)
        return ENTRY_NONE;

    valuelength = length - typelength - 1;

    if (valuelength < size)
        cstring_write_fmt(out, size, 0, "%w\\0", data + typelength + 1, &valuelength);

    return (typelength == 7 && buffer_match(data, "dynamic", 7)) ? ENTRY_DYNAMIC : ENTRY_STATIC;

}

/* the option's value, else the networks entry whose device is this one; a dynamic entry takes its lease when there is one */
char *system_address(char *device, char *option, char *key, char *out, unsigned int size)
{

    char *value = option_getstring(option);

    if (!value || !cstring_length(value))
    {

        out[0] = '\0';

        if (readentry("initrd:data/config/network.mq", device, key, out, size) == ENTRY_DYNAMIC)
            readentry(SYSTEM_LEASE, device, key, out, size);

        return out;

    }

    return value;

}

unsigned int system_unixtime(char *service)
{

    unsigned int clock = channel_lookup(service);

    if (clock)
    {

        struct message_clockinfo clockinfo;

        channel_send(1, clock, MESSAGE_TYPE_INFO, 0, 0);

        if (!channel_wait(1, clock, MESSAGE_TYPE_CLOCKINFO, sizeof (struct message_clockinfo), &clockinfo))
            return 0;

        return time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds);

    }

    return 0;

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

