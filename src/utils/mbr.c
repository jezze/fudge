#include <abi.h>
#include <fudge.h>
#include "write.h"

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

    unsigned char boot[446];
    struct partition partition[4];
    unsigned char signature[2];

};

void main(void)
{

    struct mbr mbr;
    unsigned int i;

    file_open(CALL_PI);
    file_readall(CALL_PI, &mbr, 512);
    file_close(CALL_PI);

    if (mbr.signature[0] != 0x55 || mbr.signature[1] != 0xAA)
        return;

    file_open(CALL_PO);

    for (i = 0; i < 4; i++)
    {

        unsigned int start = (mbr.partition[i].sectorlba[3] << 24) | (mbr.partition[i].sectorlba[2] << 16) | (mbr.partition[i].sectorlba[1] << 8) | (mbr.partition[i].sectorlba[0]);
        unsigned int sectors = (mbr.partition[i].sectortotal[3] << 24) | (mbr.partition[i].sectortotal[2] << 16) | (mbr.partition[i].sectortotal[1] << 8) | (mbr.partition[i].sectortotal[0]);

        write_dec("Partition", i);

        if (mbr.partition[i].systemid == 0)
            continue;

        write_hex("    Boot", mbr.partition[i].boot);
        write_hex("    Id", mbr.partition[i].systemid);
        write_dec("    Start", start);
        write_dec("    End", start + sectors - 1);
        write_dec("    Sectors", sectors);
        write_ratio("    Cylinder", mbr.partition[i].cylinderbase | ((mbr.partition[i].sectorbase & 0xC0) << 8), mbr.partition[i].cylinderlimit | ((mbr.partition[i].sectorlimit & 0xC0) << 8));
        write_ratio("    Head", mbr.partition[i].headbase, mbr.partition[i].headlimit);
        write_ratio("    Sector", mbr.partition[i].sectorbase & 0x2F, mbr.partition[i].sectorlimit & 0x2F);

    }

    file_close(CALL_PO);

}

