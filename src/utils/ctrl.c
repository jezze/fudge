#include <abi.h>
#include <fudge.h>
#include <common.h>

static char *names[32];
static void (*writers[32])(struct ctrl_header *header);

static void writeheader(struct ctrl_header *header)
{

    union echo_arg a[1];

    a[0].s = names[header->type];

    echo(FILE_PO, "type: %s\n", a);

}

static void writeclocksettings(struct ctrl_header *header)
{

    struct ctrl_clocksettings settings;
    union echo_arg a[7];

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_clocksettings), 0);

    a[0].u = settings.seconds;
    a[1].u = settings.minutes;
    a[2].u = settings.hours;
    a[3].u = settings.weekday;
    a[4].u = settings.day;
    a[5].u = settings.month;
    a[6].u = settings.year;

    echo(FILE_PO, "seconds: %u\nminutes: %u\nhours: %u\nweekday: %u\nday: %u\nmonth: %u\nyear: %u\n", a);

}

static void writeconsettings(struct ctrl_header *header)
{

    struct ctrl_consettings settings;
    union echo_arg a[2];

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_consettings), 0);

    a[0].u = settings.interface;
    a[1].u = settings.port;

    echo(FILE_PO, "interface: %u\nport: %u\n", a);

}

static void writeconsolesettings(struct ctrl_header *header)
{

    struct ctrl_consolesettings settings;
    union echo_arg a[1];

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_consolesettings), 0);

    a[0].u = settings.scroll;

    echo(FILE_PO, "scroll: %u\n", a);

}

static void writepartsettings(struct ctrl_header *header)
{

    struct ctrl_partsettings settings;
    union echo_arg a[3];

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_partsettings), 0);

    a[0].u = settings.interface;
    a[1].u = settings.start;
    a[2].u = settings.end;

    echo(FILE_PO, "interface: %u\nstart: %u\nend: %u\n", a);

}

static void writevideosettings(struct ctrl_header *header)
{

    struct ctrl_videosettings settings;
    union echo_arg a[3];

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_videosettings), 0);

    a[0].u = settings.w;
    a[1].u = settings.h;
    a[2].u = settings.bpp;

    echo(FILE_PO, "width: %u\nheight: %u\nbpp: %u\n", a);

}

void main(void)
{

    struct ctrl_header header;

    names[CTRL_TYPE_CLOCK] = "clocksettings";
    names[CTRL_TYPE_CON] = "consettings";
    names[CTRL_TYPE_CONSOLE] = "consolesettings";
    names[CTRL_TYPE_PART] = "partsettings";
    names[CTRL_TYPE_VIDEO] = "videosettings";
    writers[CTRL_TYPE_CLOCK] = writeclocksettings;
    writers[CTRL_TYPE_CON] = writeconsettings;
    writers[CTRL_TYPE_CONSOLE] = writeconsolesettings;
    writers[CTRL_TYPE_PART] = writepartsettings;
    writers[CTRL_TYPE_VIDEO] = writevideosettings;

    file_open(FILE_PO);
    file_open(FILE_PI);
    file_readall(FILE_PI, &header, sizeof (struct ctrl_header));
    writeheader(&header);
    writers[header.type](&header);
    file_close(FILE_PI);
    file_close(FILE_PO);

}

