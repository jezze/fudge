#include <abi.h>
#include <fudge.h>

union settings
{

    unsigned char raw[FUDGE_BSIZE];
    struct ctrl_header header;
    struct ctrl_clocksettings clocksettings;
    struct ctrl_consolesettings consolesettings;
    struct ctrl_videosettings videosettings;

};

static unsigned int writestring(char *value)
{

    return file_writeall(CALL_PO, value, ascii_length(value));

}

static unsigned int writeboolean(unsigned int value)
{

    return (value) ? writestring("yes") : writestring("no");

}

static unsigned int writedec(unsigned int value)
{

    char num[32];

    return file_writeall(CALL_PO, num, ascii_wvalue(num, 32, value, 10, 0));

}

static void writeheader(struct ctrl_header *header)
{

    writestring("type: ");

    switch (header->type)
    {

    case CTRL_TYPE_CLOCK:
        writestring("clocksettings");

        break;

    case CTRL_TYPE_CONSOLE:
        writestring("consolesettings");

        break;

    case CTRL_TYPE_VIDEO:
        writestring("videosettings");

        break;

    default:
        writestring("<null>");

        break;

    }

    writestring("\n");

}

static void writeclocksettings(struct ctrl_clocksettings *settings)
{

    writestring("seconds: ");
    writedec(settings->seconds);
    writestring("\n");
    writestring("minutes: ");
    writedec(settings->minutes);
    writestring("\n");
    writestring("hours: ");
    writedec(settings->hours);
    writestring("\n");
    writestring("weekday: ");
    writedec(settings->weekday);
    writestring("\n");
    writestring("day: ");
    writedec(settings->day);
    writestring("\n");
    writestring("month: ");
    writedec(settings->month);
    writestring("\n");
    writestring("year: ");
    writedec(settings->year);
    writestring("\n");

}

static void writeconsolesettings(struct ctrl_consolesettings *settings)
{

    writestring("scroll: ");
    writeboolean(settings->scroll);
    writestring("\n");

}

static void writevideosettings(struct ctrl_videosettings *settings)
{

    writestring("width: ");
    writedec(settings->w);
    writestring("\n");
    writestring("height: ");
    writedec(settings->h);
    writestring("\n");
    writestring("bpp: ");
    writedec(settings->bpp);
    writestring("\n");

}

void main(void)
{

    union settings buffer;

    call_open(CALL_PO);
    call_open(CALL_PI);
    file_readall(CALL_PI, &buffer.header, sizeof (struct ctrl_header));
    writeheader(&buffer.header);

    switch (buffer.header.type)
    {

    case CTRL_TYPE_CLOCK:
        file_seekreadall(CALL_PI, &buffer.clocksettings, sizeof (struct ctrl_clocksettings), 0);
        writeclocksettings(&buffer.clocksettings);

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

    call_close(CALL_PI);
    call_close(CALL_PO);

}

