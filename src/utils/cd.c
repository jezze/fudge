#include <fudge.h>
#include <abi.h>

static unsigned int eachchar(char *data, unsigned int length, unsigned int offset, char c)
{

    unsigned int i;

    for (i = offset; i < length; i++)
    {

        if (data[i] == c || data[i] == '\0')
            return i + 1 - offset;

    }

    return 0;

}

static unsigned int checkpath(char *path, unsigned int count)
{

    return (count >= 2 && path[count - 1] == '\0' && ((path[count - 2] == '/') || (path[count - 2] == ':')));

}

static unsigned int cleanpath(char *path, unsigned int count)
{

    unsigned int start = eachchar(path, count, 0, ':');
    unsigned int offset;
    unsigned int length;

    for (offset = start; (length = eachchar(path, count, offset, '/')); offset += length)
    {

        unsigned int nextoffset = offset + length;

        if (length == 1 && buffer_match(path + offset, "/", 1))
        {

            buffer_copy(path + start, "", 1);

            return start + 1;

        }

        else if (length == 2 && buffer_match(path + offset, ".", 1))
        {

            count -= nextoffset;

            buffer_copy(path + offset, path + nextoffset, count);

            length = 0;

        }

        else if (length == 3 && buffer_match(path + offset, "..", 2))
        {

            count -= nextoffset;

            buffer_copy(path + offset, path + nextoffset, count);

            length = 0;

        }

        else
        {

            unsigned int nextlength = eachchar(path, count, nextoffset, '/');

            if (nextlength == 3 && buffer_match(path + nextoffset, "..", 2))
            {

                nextoffset += nextlength;
                count -= nextoffset;

                buffer_copy(path + offset, path + nextoffset, count);

                length = 0;
                offset = start;

            }

        }

    }

    return count;

}

static void onpath(struct message *message)
{

    if (checkpath(message->data, message->length))
    {

        message->length = cleanpath(message->data, message->length);

        if (message->length)
        {

            unsigned int target = fs_auth(message->data);

            if (target)
            {

                unsigned int id = fs_walk(1, target, 0, message->data);

                if (id)
                    channel_send_fmt1(0, message->source, EVENT_OPTION, "pwd=%s\n", message->data);
                else
                    channel_send_fmt1(0, message->source, EVENT_ERROR, "Directory not found: %s\n", message->data);

            }

            else
            {

                channel_send_fmt1(0, message->source, EVENT_ERROR, "Service not found: %s\n", message->data);

            }

        }

    }

    else
    {

        channel_send_fmt1(0, message->source, EVENT_ERROR, "Directory not found: %s\n", message->data);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

