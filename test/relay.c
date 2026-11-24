#include <fudge.h>
#include <abi.h>

/* the real tr, wired as sh would wire it; the page the source fills must reach this end where it was filled, never copied */
#define BYTES                           (PAGE_SIZE * PAGE_TASKLIMIT * 4 + 123)
#define STAGES                          2

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static void setoption(unsigned int target, char *key, unsigned int value)
{

    char text[16];

    cstring_write_fmt(text, 16, 0, "%u\\0", &value);
    channel_sendoption(1, target, key, text);

}

static unsigned char expected(unsigned int at)
{

    unsigned char c = at;

    return (c == 'a') ? 'b' : c;

}

static void onmain(struct message *message)
{

    unsigned int source = fs_spawn(1, "initrd:test/source");
    unsigned int tr = fs_spawn(1, "initrd:bin/tr");
    unsigned int bytes = BYTES;
    unsigned int report = 1;
    unsigned int reported = 0;
    unsigned int first = 0;
    unsigned int position = 0;
    unsigned int wrong = 0;
    unsigned int done = 0;
    struct message_pipe pipe;
    struct message current;

    if (!source || !tr)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "spawn failed\n");

        return;

    }

    setoption(source, "target", tr);
    setoption(source, "bytes", bytes);
    setoption(source, "report", report);
    channel_sendoption(1, tr, "f", "a");
    channel_sendoption(1, tr, "t", "b");
    message_initpipe(&pipe);

    pipe.prev = source;
    pipe.nroutes = 1;
    pipe.routes[0].event = MESSAGE_TYPE_DATA;
    pipe.routes[0].to = MESSAGE_TYPE_DATA;
    pipe.routes[0].target = 0;

    channel_send(1, tr, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
    message_initpipe(&pipe);

    /* tr ends when its input does, so the source tells it on exit, as the stage before it in a real pipe */
    pipe.nterms = 1;
    pipe.terms[0].event = MESSAGE_TYPE_TERM;
    pipe.terms[0].to = MESSAGE_TYPE_TERM;
    pipe.terms[0].target = tr;

    channel_send(1, source, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
    channel_send(1, source, MESSAGE_TYPE_TERM, 0, 0);

    while (done < STAGES && channel_pick(1, &current))
    {

        if (current.source != source && current.source != tr)
            continue;

        switch (current.type)
        {

        case MESSAGE_TYPE_DATA:
            /* the address comes first; the source also says when it waits for pages */
            if (current.source == source)
            {

                if (!reported)
                    reported = cstring_read_value(current.data, current.length, 10);

            }

            else
            {

                struct block *block = current.data;
                unsigned int i;

                if (!position)
                    first = (unsigned long)current.data;

                for (i = 0; i < current.length; i++)
                {

                    if (block->bytes[i] != expected(position + i))
                        wrong++;

                }

                position += current.length;

            }

            break;

        case MESSAGE_TYPE_ERROR:
            channel_send(0, message->source, MESSAGE_TYPE_ERROR, current.length, current.data);

            break;

        case MESSAGE_TYPE_STATUS:
            done++;

            break;

        }

    }

    if (position != bytes || wrong)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%u of %u bytes, %u wrong\n", &position, &bytes, &wrong);

    if (!reported || first != reported)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "the first page was copied: given at %u, arrived at %u\n", &reported, &first);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

