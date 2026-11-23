#include <fudge.h>
#include <abi.h>

/* runs one real pipeline round after round; every round must give what the first gave; many small pages, as from ls, once showed a TERM overtaking them */
#define COMMANDS                        4
#define ROUNDS                          20
#define OUTPUTSIZE                      4096

static char *commands[COMMANDS] = {
    "ls initrd:bin | wc",
    "ls initrd:bin",
    "echo initrd:bin/wm | wc",
    "echo initrd:data/help.txt | tr -f e -t E"
};

static char first[OUTPUTSIZE];

static void flatten(char *text, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (text[i] == '\n')
            text[i] = '/';

    }

}
static char output[OUTPUTSIZE];

static void onmain(struct message *message)
{

    unsigned int index = option_getdecimal("command") % COMMANDS;
    unsigned int expected = system_feed(0, 0, first, OUTPUTSIZE, commands[index]);
    unsigned int round;


    if (!expected)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%s gave nothing\n", commands[index]);

        return;

    }

    for (round = 1; round < ROUNDS; round++)
    {

        unsigned int count = system_feed(0, 0, output, OUTPUTSIZE, commands[index]);

        if (count != expected || !buffer_match(output, first, count))
        {

            struct page_handoff handoff;
            unsigned int late = 0;

            while (call_pagetake(1, &handoff, 0) == MESSAGE_OK)
                late++;

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%u pages were still queued after the status\n", &late);

            flatten(first, expected);
            flatten(output, count);
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%s differed in round %u: first [%w] now [%w]\n", commands[index], &round, first, &expected, output, &count);

            return;

        }

    }

}

void init(void)
{

    option_add("command", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

