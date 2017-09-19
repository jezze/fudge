#include <abi.h>
#include <fudge.h>
#include "write.h"

static char *names[32];
static void (*writers[32])(struct ctrl_header *header);

static void writeheader(struct ctrl_header *header)
{

    write_string("type", names[header->type]);

}

static void writeclocksettings(struct ctrl_header *header)
{

    struct ctrl_clocksettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_clocksettings), 0);
    write_dec("seconds", settings.seconds);
    write_dec("minutes", settings.minutes);
    write_dec("hours", settings.hours);
    write_dec("weekday", settings.weekday);
    write_dec("day", settings.day);
    write_dec("month", settings.month);
    write_dec("year", settings.year);

}

static void writeconsettings(struct ctrl_header *header)
{

    struct ctrl_consettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_consettings), 0);
    write_dec("interface", settings.interface);
    write_dec("port", settings.port);

}

static void writeconsolesettings(struct ctrl_header *header)
{

    struct ctrl_consolesettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_consolesettings), 0);
    write_boolean("scroll", settings.scroll);

}

static void writepartsettings(struct ctrl_header *header)
{

    struct ctrl_partsettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_partsettings), 0);
    write_dec("interface", settings.interface);
    write_dec("start", settings.start);
    write_dec("end", settings.end);

}

static void writevideosettings(struct ctrl_header *header)
{

    struct ctrl_videosettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_videosettings), 0);
    write_dec("width", settings.w);
    write_dec("height", settings.h);
    write_dec("bpp", settings.bpp);

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

