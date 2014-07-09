#include <fudge.h>

struct partition
{

    unsigned char boot;
    unsigned char headbase;
    unsigned char sectorbase;
    unsigned char cylinderbase;
    unsigned char systemid;
    unsigned char headlimit;
    unsigned char sectorlimit;
    unsigned char cylinderlimit;
    unsigned char sectorlba[4];
    unsigned char sectortotal[4];

};

struct mbr
{

    unsigned char boot[440];
    unsigned char id[4];
    unsigned char protected[2];
    struct partition partition[4];
    unsigned char signature[2];

};

void main()
{

    struct mbr mbr;
    unsigned int i;
    unsigned int woff = 0;

    call_open(CALL_I0, CALL_OPEN_READ);
    call_read(CALL_I0, 0, 512, &mbr);
    call_close(CALL_I0);

    if (mbr.signature[0] != 0x55 || mbr.signature[1] != 0xAA)
        return;

    call_open(CALL_O0, CALL_OPEN_WRITE);
    woff += call_write(CALL_O0, woff, 4, "Id: ");
    woff += call_write(CALL_O0, woff, 4, &mbr.id);
    woff += call_write(CALL_O0, woff, 1, "\n");

    for (i = 0; i < 4; i++)
    {

        char num[32];
        unsigned int count;

        if (mbr.partition[i].systemid == 0)
            continue;

        woff += call_write(CALL_O0, woff, 10, "Partition ");
        count = ascii_write_value(num, 32, i, 10, 0);
        woff += call_write(CALL_O0, woff, count, num);
        woff += call_write(CALL_O0, woff, 2, ":\n");
        woff += call_write(CALL_O0, woff, 11, "    Boot 0x");
        count = ascii_write_value(num, 32, mbr.partition[i].boot, 16, 0);
        woff += call_write(CALL_O0, woff, count, num);
        woff += call_write(CALL_O0, woff, 1, "\n");
        woff += call_write(CALL_O0, woff, 15, "    Systemid 0x");
        count = ascii_write_value(num, 32, mbr.partition[i].systemid, 16, 0);
        woff += call_write(CALL_O0, woff, count, num);
        woff += call_write(CALL_O0, woff, 1, "\n");

    }

    call_close(CALL_O0);

}

