#include <abi.h>
#include <fudge.h>

union settings
{

    unsigned char raw[FUDGE_BSIZE];
    struct ctrl_header header;
    struct ctrl_consolesettings consolesettings;
    struct ctrl_videosettings videosettings;

};

static unsigned int writestring(unsigned int offset, char *value)
{

    return call_write(CALL_PO, offset, ascii_length(value), 1, value);

}

static unsigned int writeboolean(unsigned int offset, unsigned int value)
{

    if (value)
        return writestring(offset, "yes");
    else
        return writestring(offset, "no");

}

static unsigned int writedec(unsigned int offset, unsigned int value)
{

    char num[32];
    unsigned int count = ascii_wvalue(num, 32, value, 10, 0);

    return call_write(CALL_PO, offset, count, 1, num);

}

/*
static unsigned int writehex2(unsigned int offset, unsigned char value)
{

    char num[32];
    unsigned int count = ascii_wzerovalue(num, 32, value, 16, 2, 0);

    return call_write(CALL_PO, offset, count, 1, num);

}
*/

static unsigned int writeheader(unsigned int offset, struct ctrl_header *header)
{

    offset += writestring(offset, "type: ");

    switch (header->type)
    {

    case CTRL_TYPE_CONSOLE:
        offset += writestring(offset, "consolesettings");

        break;

    case CTRL_TYPE_VIDEO:
        offset += writestring(offset, "videosettings");

        break;

    default:
        offset += writestring(offset, "<null>");

        break;

    }

    offset += writestring(offset, "\n");

    return offset;

}

static unsigned int writeconsolesettings(unsigned int offset, struct ctrl_consolesettings *settings)
{

    offset += writestring(offset, "scroll: ");
    offset += writeboolean(offset, settings->scroll);
    offset += writestring(offset, "\n");

    return offset;

}

static unsigned int writevideosettings(unsigned int offset, struct ctrl_videosettings *settings)
{

    offset += writestring(offset, "width: ");
    offset += writedec(offset, settings->w);
    offset += writestring(offset, "\n");
    offset += writestring(offset, "height: ");
    offset += writedec(offset, settings->h);
    offset += writestring(offset, "\n");
    offset += writestring(offset, "bpp: ");
    offset += writedec(offset, settings->bpp);
    offset += writestring(offset, "\n");

    return offset;

}

void main()
{

    union settings buffer;
    unsigned int count, roff, woff;

    call_open(CALL_P0);

    for (roff = 0; (count = call_read(CALL_P0, roff, 1, FUDGE_BSIZE, buffer.raw)); roff += count)

    call_close(CALL_P0);
    call_open(CALL_PO);

    woff = writeheader(0, &buffer.header);

    switch (buffer.header.type)
    {

    case CTRL_TYPE_CONSOLE:
        writeconsolesettings(woff, &buffer.consolesettings);

        break;

    case CTRL_TYPE_VIDEO:
        writevideosettings(woff, &buffer.videosettings);

        break;

    }

    call_close(CALL_PO);

}

