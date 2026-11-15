#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int power = channel_lookup(option_getstring("power-service"));

    if (!power)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Service not found: %s\n", option_getstring("power-service"));
    else if (!channel_send(1, power, MESSAGE_TYPE_POWEROFF, 0, 0))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not power off: %s\n", option_getstring("power-service"));

}

void init(void)
{

    option_add("power-service", "power:0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

