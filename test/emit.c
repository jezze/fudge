#include <fudge.h>
#include <abi.h>

/* odd sizes, so the pieces never line up with pages */
#define PIECE                           1000

static void onmain(struct message *message)
{

    unsigned int total = option_getdecimal("bytes");
    unsigned int sent = 0;

    while (sent < total)
    {

        char piece[PIECE];
        unsigned int count = (total - sent < PIECE) ? total - sent : PIECE;
        unsigned int i;

        for (i = 0; i < count; i++)
            piece[i] = 'a' + (sent + i) % 26;

        if (!channel_send(0, message->source, MESSAGE_TYPE_DATA, count, piece))
            return;

        sent += count;

    }

}

void init(void)
{

    option_add("bytes", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

