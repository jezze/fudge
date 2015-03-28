#include <abi.h>
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

    call_open(CALL_P0);
    call_read(CALL_P0, 0, 512, 1, &mbr);
    call_close(CALL_P0);

    if (mbr.signature[0] != 0x55 || mbr.signature[1] != 0xAA)
        return;

    call_open(CALL_PO);

    woff += call_write(CALL_PO, woff, 1, 4, "Id: ");
    woff += call_write(CALL_PO, woff, 1, 4, &mbr.id);
    woff += call_write(CALL_PO, woff, 1, 1, "\n");

    for (i = 0; i < 4; i++)
    {

        char num[32];
        unsigned int count;

        woff += call_write(CALL_PO, woff, 1, 10, "Partition ");
        count = ascii_wvalue(num, 32, i, 10, 0);
        woff += call_write(CALL_PO, woff, 1, count, num);
        woff += call_write(CALL_PO, woff, 1, 2, ":\n");

        if (mbr.partition[i].systemid == 0)
            continue;

        woff += call_write(CALL_PO, woff, 1, 11, "    Boot 0x");
        count = ascii_wvalue(num, 32, mbr.partition[i].boot, 16, 0);
        woff += call_write(CALL_PO, woff, 1, count, num);
        woff += call_write(CALL_PO, woff, 1, 1, "\n");
        woff += call_write(CALL_PO, woff, 1, 15, "    Systemid 0x");
        count = ascii_wvalue(num, 32, mbr.partition[i].systemid, 16, 0);
        woff += call_write(CALL_PO, woff, 1, count, num);
        woff += call_write(CALL_PO, woff, 1, 1, "\n");

    }

    call_close(CALL_PO);

}

