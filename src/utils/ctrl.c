#include <abi.h>
#include <fudge.h>

union settings
{

    unsigned char raw[FUDGE_BSIZE];
    struct ctrl_header header;
    struct ctrl_clocksettings clocksettings;
    struct ctrl_consettings consettings;
    struct ctrl_consolesettings consolesettings;
    struct ctrl_videosettings videosettings;

};

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

    char *key = "type";

    switch (header->type)
    {

    case CTRL_TYPE_CLOCK:
        writestring(key, "clocksettings");

        break;

    case CTRL_TYPE_CON:
        writestring(key, "consettings");

        break;

    case CTRL_TYPE_CONSOLE:
        writestring(key, "consolesettings");

        break;

    case CTRL_TYPE_VIDEO:
        writestring(key, "videosettings");

        break;

    default:
        writestring(key, "<null>");

        break;

    }

}

static void writeclocksettings(struct ctrl_clocksettings *settings)
{

    writedec("seconds", settings->seconds);
    writedec("minutes", settings->minutes);
    writedec("hours", settings->hours);
    writedec("weekday", settings->weekday);
    writedec("day", settings->day);
    writedec("month", settings->month);
    writedec("year", settings->year);

}

static void writeconsettings(struct ctrl_consettings *settings)
{

    writedec("linkprotocol", settings->linkprotocol);
    writedec("networkprotocol", settings->networkprotocol);

}

static void writeconsolesettings(struct ctrl_consolesettings *settings)
{

    writeboolean("scroll", settings->scroll);

}

static void writevideosettings(struct ctrl_videosettings *settings)
{

    writedec("width", settings->w);
    writedec("height", settings->h);
    writedec("bpp", settings->bpp);

}

void main(void)
{

    union settings buffer;

    file_open(CALL_PO);
    file_open(CALL_PI);
    file_readall(CALL_PI, &buffer.header, sizeof (struct ctrl_header));
    writeheader(&buffer.header);

    switch (buffer.header.type)
    {

    case CTRL_TYPE_CLOCK:
        file_seekreadall(CALL_PI, &buffer.clocksettings, sizeof (struct ctrl_clocksettings), 0);
        writeclocksettings(&buffer.clocksettings);

        break;

    case CTRL_TYPE_CON:
        file_seekreadall(CALL_PI, &buffer.consettings, sizeof (struct ctrl_consettings), 0);
        writeconsettings(&buffer.consettings);

        break;

    case CTRL_TYPE_CONSOLE:
        file_seekreadall(CALL_PI, &buffer.consolesettings, sizeof (struct ctrl_consolesettings), 0);
        writeconsolesettings(&buffer.consolesettings);

        break;

    case CTRL_TYPE_VIDEO:
        file_seekreadall(CALL_PI, &buffer.videosettings, sizeof (struct ctrl_videosettings), 0);
        writevideosettings(&buffer.videosettings);

        break;

    }

    file_close(CALL_PI);
    file_close(CALL_PO);

}

