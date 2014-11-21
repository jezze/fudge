#include <fudge.h>

union ctrl
{

    unsigned char raw[FUDGE_BSIZE];
    struct ctrl_header header;
    struct ctrl_consolectrl consolectrl;
    struct ctrl_networkctrl networkctrl;
    struct ctrl_videoctrl videoctrl;

};

static unsigned int writestring(unsigned int offset, char *value)
{

    return call_write(CALL_O0, offset, ascii_length(value), value);

}

static unsigned int writeboolean(unsigned int offset, unsigned int value)
{

    if (value)
        return writestring(offset, "yes");
    else
        return writestring(offset, "no");

}

static unsigned int writedec(unsigned int offset, unsigned char value)
{

    char num[32];
    unsigned int count = ascii_wvalue(num, 32, value, 10, 0);

    return call_write(CALL_O0, offset, count, num);

}

static unsigned int writehex2(unsigned int offset, unsigned char value)
{

    char num[32];
    unsigned int count = ascii_wzerovalue(num, 32, value, 16, 2, 0);

    return call_write(CALL_O0, offset, count, num);

}

static unsigned int writeheader(unsigned int offset, struct ctrl_header *header)
{

    offset += writestring(offset, "type: ");

    switch (header->type)
    {

    case CTRL_TYPE_CONSOLECTRL:
        offset += writestring(offset, "consolectrl");

        break;

    case CTRL_TYPE_NETWORKCTRL:
        offset += writestring(offset, "networkctrl");

        break;

    case CTRL_TYPE_VIDEOCTRL:
        offset += writestring(offset, "videoctrl");

        break;

    default:
        offset += writestring(offset, "<null>");

        break;

    }

    offset += writestring(offset, "\n");

    return offset;

}

static unsigned int writeconsolectrl(unsigned int offset, struct ctrl_consolectrl *consolectrl)
{

    offset += writestring(offset, "scroll: ");
    offset += writeboolean(offset, consolectrl->scroll);
    offset += writestring(offset, "\n");

    return offset;

}

static unsigned int writenetworkctrl(unsigned int offset, struct ctrl_networkctrl *networkctrl)
{

    offset += writestring(offset, "mac: ");
    offset += writehex2(offset, networkctrl->mac[0]);
    offset += writestring(offset, ":");
    offset += writehex2(offset, networkctrl->mac[1]);
    offset += writestring(offset, ":");
    offset += writehex2(offset, networkctrl->mac[2]);
    offset += writestring(offset, ":");
    offset += writehex2(offset, networkctrl->mac[3]);
    offset += writestring(offset, ":");
    offset += writehex2(offset, networkctrl->mac[4]);
    offset += writestring(offset, ":");
    offset += writehex2(offset, networkctrl->mac[5]);
    offset += writestring(offset, "\n");

    return offset;

}

static unsigned int writevideoctrl(unsigned int offset, struct ctrl_videoctrl *videoctrl)
{

    offset += writestring(offset, "width: ");
    offset += writedec(offset, videoctrl->w);
    offset += writestring(offset, "\n");
    offset += writestring(offset, "height: ");
    offset += writedec(offset, videoctrl->h);
    offset += writestring(offset, "\n");
    offset += writestring(offset, "bpp: ");
    offset += writedec(offset, videoctrl->bpp);
    offset += writestring(offset, "\n");

    return offset;

}

void main()
{

    union ctrl buffer;
    unsigned int count, roff, woff;

    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer.raw)); roff += count)

    call_close(CALL_I0);
    call_open(CALL_O0);

    woff = writeheader(0, &buffer.header);

    switch (buffer.header.type)
    {

    case CTRL_TYPE_CONSOLECTRL:
        writeconsolectrl(woff, &buffer.consolectrl);

        break;

    case CTRL_TYPE_NETWORKCTRL:
        writenetworkctrl(woff, &buffer.networkctrl);

        break;

    case CTRL_TYPE_VIDEOCTRL:
        writevideoctrl(woff, &buffer.videoctrl);

        break;

    }

    call_close(CALL_O0);

}

