#include <fudge.h>
#include <abi.h>

static void printvalue(unsigned int source, char *name, unsigned int value, char *unit)
{

    if (value != MESSAGE_BATTERY_UNKNOWN)
        channel_send_fmt(0, source, MESSAGE_TYPE_DATA, "%s: %u %s\n", name, &value, unit);

}

static void onmain(struct message *message)
{

    unsigned int battery = channel_lookup(option_getstring("battery-service"));

    if (battery)
    {

        struct message_batteryinfo batteryinfo;

        channel_send(1, battery, MESSAGE_TYPE_INFO, 0, 0);

        if (channel_wait(1, battery, MESSAGE_TYPE_BATTERYINFO, sizeof (struct message_batteryinfo), &batteryinfo))
        {

            char *charge = (batteryinfo.unit == MESSAGE_BATTERY_MILLIAMPS) ? "mAh" : "mWh";
            char *rate = (batteryinfo.unit == MESSAGE_BATTERY_MILLIAMPS) ? "mA" : "mW";

            if (!batteryinfo.present)
            {

                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "Not present\n");

                return;

            }

            if (batteryinfo.remaining != MESSAGE_BATTERY_UNKNOWN && batteryinfo.full != MESSAGE_BATTERY_UNKNOWN && batteryinfo.full)
            {

                unsigned int percent = batteryinfo.remaining * 100 / batteryinfo.full;

                if (percent > 100)
                    percent = 100;

                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "Charge: %u%s\n", &percent, "%");

            }

            if (batteryinfo.state & MESSAGE_BATTERY_CHARGING)
                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "State: charging\n");
            else if (batteryinfo.state & MESSAGE_BATTERY_DISCHARGING)
                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "State: discharging\n");
            else
                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "State: idle\n");

            if (batteryinfo.state & MESSAGE_BATTERY_CRITICAL)
                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "Critical\n");

            printvalue(message->source, "Remaining", batteryinfo.remaining, charge);
            printvalue(message->source, "Full", batteryinfo.full, charge);
            printvalue(message->source, "Design", batteryinfo.design, charge);
            printvalue(message->source, "Rate", batteryinfo.rate, rate);
            printvalue(message->source, "Voltage", batteryinfo.voltage, "mV");

        }

    }

    else
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Service not found: %s\n", option_getstring("battery-service"));

    }

}

void init(void)
{

    option_add("battery-service", "battery:0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

