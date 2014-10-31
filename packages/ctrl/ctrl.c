#include <fudge.h>

union ctrl
{

    unsigned char raw[FUDGE_BSIZE];
    struct ctrl_header header;
    struct ctrl_networkctrl networkctrl;
    struct ctrl_videoctrl videoctrl;

};

void writeheader(struct ctrl_header *header)
{

    unsigned int woff = 0;

    call_open(CALL_O0);

    woff += call_write(CALL_O0, woff, 6, "type: ");

    switch (header->type)
    {

    case CTRL_TYPE_NETWORKCTRL:
        woff += call_write(CALL_O0, woff, 11, "networkctrl");

        break;

    case CTRL_TYPE_VIDEOCTRL:
        woff += call_write(CALL_O0, woff, 9, "videoctrl");

        break;

    default:
        woff += call_write(CALL_O0, woff, 6, "<null>");

        break;

    }

    woff += call_write(CALL_O0, woff, 1, "\n");

    call_close(CALL_O0);

}

void writenetworkctrl(struct ctrl_networkctrl *networkctrl)
{

    char num[32];
    unsigned int count;
    unsigned int woff = 0;

    call_open(CALL_O0);

    woff += call_write(CALL_O0, woff, 5, "mac: ");
    count = ascii_wzerovalue(num, 32, networkctrl->mac[0], 16, 2, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, ":");
    count = ascii_wzerovalue(num, 32, networkctrl->mac[1], 16, 2, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, ":");
    count = ascii_wzerovalue(num, 32, networkctrl->mac[2], 16, 2, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, ":");
    count = ascii_wzerovalue(num, 32, networkctrl->mac[3], 16, 2, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, ":");
    count = ascii_wzerovalue(num, 32, networkctrl->mac[4], 16, 2, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, ":");
    count = ascii_wzerovalue(num, 32, networkctrl->mac[5], 16, 2, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, "\n");

    call_close(CALL_O0);

}

void writevideoctrl(struct ctrl_videoctrl *videoctrl)
{

    char num[32];
    unsigned int count;
    unsigned int woff = 0;

    call_open(CALL_O0);

    woff += call_write(CALL_O0, woff, 7, "width: ");
    count = ascii_wvalue(num, 32, videoctrl->w, 10, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, "\n");
    woff += call_write(CALL_O0, woff, 8, "height: ");
    count = ascii_wvalue(num, 32, videoctrl->h, 10, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, "\n");
    woff += call_write(CALL_O0, woff, 6, "bpp: ");
    count = ascii_wvalue(num, 32, videoctrl->bpp, 10, 0);
    woff += call_write(CALL_O0, woff, count, num);
    woff += call_write(CALL_O0, woff, 1, "\n");

    call_close(CALL_O0);

}

void main()
{

    union ctrl buffer;
    unsigned int count, roff;

    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer.raw)); roff += count)

    call_close(CALL_I0);

    switch (buffer.header.type)
    {

    case CTRL_TYPE_NETWORKCTRL:
        writeheader(&buffer.header);
        writenetworkctrl(&buffer.networkctrl);

        break;

    case CTRL_TYPE_VIDEOCTRL:
        writeheader(&buffer.header);
        writevideoctrl(&buffer.videoctrl);

        break;

    default:
        writeheader(&buffer.header);

        break;

    }

}

