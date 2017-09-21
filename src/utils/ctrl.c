#include <abi.h>
#include <fudge.h>
#include <common.h>

static char *names[32];
static void (*writers[32])(struct ctrl_header *header);

static void writeheader(struct ctrl_header *header)
{

    write_keystring("type", names[header->type]);

}

static void writeclocksettings(struct ctrl_header *header)
{

    struct ctrl_clocksettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_clocksettings), 0);
    write_keydec("seconds", settings.seconds);
    write_keydec("minutes", settings.minutes);
    write_keydec("hours", settings.hours);
    write_keydec("weekday", settings.weekday);
    write_keydec("day", settings.day);
    write_keydec("month", settings.month);
    write_keydec("year", settings.year);

}

static void writeconsettings(struct ctrl_header *header)
{

    struct ctrl_consettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_consettings), 0);
    write_keydec("interface", settings.interface);
    write_keydec("port", settings.port);

}

static void writeconsolesettings(struct ctrl_header *header)
{

    struct ctrl_consolesettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_consolesettings), 0);
    write_keyboolean("scroll", settings.scroll);

}

static void writepartsettings(struct ctrl_header *header)
{

    struct ctrl_partsettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_partsettings), 0);
    write_keydec("interface", settings.interface);
    write_keydec("start", settings.start);
    write_keydec("end", settings.end);

}

static void writevideosettings(struct ctrl_header *header)
{

    struct ctrl_videosettings settings;

    file_seekreadall(FILE_PI, &settings, sizeof (struct ctrl_videosettings), 0);
    write_keydec("width", settings.w);
    write_keydec("height", settings.h);
    write_keydec("bpp", settings.bpp);

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

