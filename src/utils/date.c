#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_clocksettings settings;
    char *datetime = "0000-00-00 00:00:00\n";

    if (!file_walk(FILE_L0, "/system/clock/if:0/ctrl"))
        return;

    file_open(FILE_L0);
    file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
    file_close(FILE_L0);
    ascii_wzerovalue(datetime, 20, settings.year, 10, 4, 0);
    ascii_wzerovalue(datetime, 20, settings.month, 10, 2, 5);
    ascii_wzerovalue(datetime, 20, settings.day, 10, 2, 8);
    ascii_wzerovalue(datetime, 20, settings.hours, 10, 2, 11);
    ascii_wzerovalue(datetime, 20, settings.minutes, 10, 2, 14);
    ascii_wzerovalue(datetime, 20, settings.seconds, 10, 2, 17);
    file_open(FILE_PO);
    file_writeall(FILE_PO, datetime, ascii_length(datetime));
    file_close(FILE_PO);

}

