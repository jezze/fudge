#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int clock = channel_lookup(option_getstring("clock-service"));

    if (clock)
    {

        struct message_clockinfo clockinfo;

        channel_send(1, clock, MESSAGE_TYPE_INFO, 0, 0);
        channel_wait(1, clock, MESSAGE_TYPE_CLOCKINFO, sizeof (struct message_clockinfo), &clockinfo);
        channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "%4h-%2c-%2c %2c:%2c:%2c\n", &clockinfo.year, &clockinfo.month, &clockinfo.day, &clockinfo.hours, &clockinfo.minutes, &clockinfo.seconds);

    }

}

void init(void)
{

    option_add("clock-service", "clock:0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

