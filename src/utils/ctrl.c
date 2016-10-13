#include <abi.h>
#include <fudge.h>

static char *names[32];
static void (*writers[32])(struct ctrl_header *header);

static void writebuffer(char *key, char *value, unsigned int count)
{

    file_writeall(CALL_PO, key, ascii_length(key));
    file_writeall(CALL_PO, ": ", 2);
    file_writeall(CALL_PO, value, count);
    file_writeall(CALL_PO, "\n", 1);

}

static void writestring(char *key, char *value)
{

    writebuffer(key, value, ascii_length(value));

}

static void writeboolean(char *key, unsigned int value)
{

    if (value)
        writebuffer(key, "true", 4);
    else
        writebuffer(key, "false", 5);

}

static void writedec(char *key, unsigned int value)
{

    char num[FUDGE_NSIZE];

    writebuffer(key, num, ascii_wvalue(num, FUDGE_NSIZE, value, 10, 0));

}

static void writeheader(struct ctrl_header *header)
{

    writestring("type", names[header->type]);

}

static void writeclocksettings(struct ctrl_header *header)
{

    struct ctrl_clocksettings settings;

    file_seekreadall(CALL_PI, &settings, sizeof (struct ctrl_clocksettings), 0);
    writedec("seconds", settings.seconds);
    writedec("minutes", settings.minutes);
    writedec("hours", settings.hours);
    writedec("weekday", settings.weekday);
    writedec("day", settings.day);
    writedec("month", settings.month);
    writedec("year", settings.year);

}

static void writeconsettings(struct ctrl_header *header)
{

    struct ctrl_consettings settings;

    file_seekreadall(CALL_PI, &settings, sizeof (struct ctrl_consettings), 0);
    writedec("interface", settings.interface);
    writedec("port", settings.port);

}

static void writeconsolesettings(struct ctrl_header *header)
{

    struct ctrl_consolesettings settings;

    file_seekreadall(CALL_PI, &settings, sizeof (struct ctrl_consolesettings), 0);
    writeboolean("scroll", settings.scroll);

}

static void writepartsettings(struct ctrl_header *header)
{

    struct ctrl_partsettings settings;

    file_seekreadall(CALL_PI, &settings, sizeof (struct ctrl_partsettings), 0);
    writedec("interface", settings.interface);
    writedec("start", settings.start);
    writedec("end", settings.end);

}

static void writevideosettings(struct ctrl_header *header)
{

    struct ctrl_videosettings settings;

    file_seekreadall(CALL_PI, &settings, sizeof (struct ctrl_videosettings), 0);
    writedec("width", settings.w);
    writedec("height", settings.h);
    writedec("bpp", settings.bpp);

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

    file_open(CALL_PO);
    file_open(CALL_PI);
    file_readall(CALL_PI, &header, sizeof (struct ctrl_header));
    writeheader(&header);
    writers[header.type](&header);
    file_close(CALL_PI);
    file_close(CALL_PO);

}

