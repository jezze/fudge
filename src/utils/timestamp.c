#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fsp_auth(option_getstring("clock"));

    if (service)
    {

        unsigned int id = fsp_walk(service, fsp_walk(service, 0, option_getstring("clock")), "ctrl");

        if (id)
        {

            struct ctrl_clocksettings settings;
            unsigned int timestamp;

            fsp_read_all(service, id, &settings, sizeof (struct ctrl_clocksettings), 0);

            timestamp = time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds);

            channel_send_fmt1(source, EVENT_DATA, "%u\n", &timestamp);

        }

    }

}

void init(void)
{

    option_add("clock", "system:clock/if.0");
    channel_bind(EVENT_MAIN, onmain);

}

