#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_clocksettings settings;
    char *datetime = "0000-00-00 00:00:00\n";

    if (!file_walk(CALL_L0, "/system/clock/if:0/ctrl"))
        return;

    file_open(CALL_L0);
    file_readall(CALL_L0, &settings, sizeof (struct ctrl_clocksettings));
    file_close(CALL_L0);
    ascii_wzerovalue(datetime, 20, settings.year, 10, 4, 0);
    ascii_wzerovalue(datetime, 20, settings.month, 10, 2, 5);
    ascii_wzerovalue(datetime, 20, settings.day, 10, 2, 8);
    ascii_wzerovalue(datetime, 20, settings.hours, 10, 2, 11);
    ascii_wzerovalue(datetime, 20, settings.minutes, 10, 2, 14);
    ascii_wzerovalue(datetime, 20, settings.seconds, 10, 2, 17);
    file_open(CALL_PO);
    file_writeall(CALL_PO, datetime, ascii_length(datetime));
    file_close(CALL_PO);

}

