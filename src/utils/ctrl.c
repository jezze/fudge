#include <abi.h>
#include <fudge.h>

union settings
{

    unsigned char raw[FUDGE_BSIZE];
    struct ctrl_header header;
    struct ctrl_consolesettings consolesettings;
    struct ctrl_videosettings videosettings;

};

static unsigned int writestring(char *value)
{

    return call_write(CALL_PO, ascii_length(value), value);

}

static unsigned int writeboolean(unsigned int value)
{

    return (value) ? writestring("yes") : writestring("no");

}

static unsigned int writedec(unsigned int value)
{

    char num[32];

    return call_write(CALL_PO, ascii_wvalue(num, 32, value, 10, 0), num);

}

/*
static unsigned int writehex2(unsigned int offset, unsigned char value)
{

    char num[32];
    unsigned int count = ascii_wzerovalue(num, 32, value, 16, 2, 0);

    return call_write(CALL_PO, offset, count, 1, num);

}
*/

static void writeheader(struct ctrl_header *header)
{

    writestring("type: ");

    switch (header->type)
    {

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
    call_open(CALL_P0);
    call_read(CALL_P0, sizeof (struct ctrl_header), &buffer.header);

    writeheader(&buffer.header);

    switch (buffer.header.type)
    {

    case CTRL_TYPE_CONSOLE:
        call_read(CALL_P0, sizeof (struct ctrl_consolesettings), &buffer.consolesettings);
        writeconsolesettings(&buffer.consolesettings);

        break;

    case CTRL_TYPE_VIDEO:
        call_read(CALL_P0, sizeof (struct ctrl_videosettings), &buffer.videosettings);
        writevideosettings(&buffer.videosettings);

        break;

    }

    call_close(CALL_P0);
    call_close(CALL_PO);

}

