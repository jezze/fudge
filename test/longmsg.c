#include <fudge.h>
#include <abi.h>

#define LONG                            250

static void onmain(struct message *message)
{

    char text[LONG + 1];

    buffer_clear(text, LONG + 1);
    buffer_write(text, LONG, "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", LONG, 0);
    channel_sendoption(0, message->source, "key", text);
    channel_sendpath(0, message->source, text);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

