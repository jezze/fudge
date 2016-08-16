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

        char num[FUDGE_NSIZE];
        unsigned int start = (mbr.partition[i].sectorlba[3] << 24) | (mbr.partition[i].sectorlba[2] << 16) | (mbr.partition[i].sectorlba[1] << 8) | (mbr.partition[i].sectorlba[0]);
        unsigned int sectors = (mbr.partition[i].sectortotal[3] << 24) | (mbr.partition[i].sectortotal[2] << 16) | (mbr.partition[i].sectortotal[1] << 8) | (mbr.partition[i].sectortotal[0]);

        file_writeall(CALL_PO, "Partition ", 10);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, i, 10, 0));
        file_writeall(CALL_PO, ":\n", 2);

        if (mbr.partition[i].systemid == 0)
        {

            file_writeall(CALL_PO, "    (none)", 10);
            file_writeall(CALL_PO, "\n", 1);

            continue;

        }

        file_writeall(CALL_PO, "    Boot: 0x", 12);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].boot, 16, 0));
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    Id: 0x", 10);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].systemid, 16, 0));
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    Start: ", 11);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, start, 10, 0));
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    End: ", 9);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, start + sectors - 1, 10, 0));
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    Sectors: ", 13);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, sectors, 10, 0));
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    Cylinder: ", 14);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].cylinderbase | ((mbr.partition[i].sectorbase & 0xC0) << 8), 10, 0));
        file_writeall(CALL_PO, "/", 1);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].cylinderlimit | ((mbr.partition[i].sectorlimit & 0xC0) << 8), 10, 0));
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    Head: ", 10);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].headbase, 10, 0));
        file_writeall(CALL_PO, "/", 1);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].headlimit, 10, 0));
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    Sector: ", 12);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].sectorbase & 0x2F, 10, 0));
        file_writeall(CALL_PO, "/", 1);
        file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, mbr.partition[i].sectorlimit & 0x2F, 10, 0));
        file_writeall(CALL_PO, "\n", 1);

    }

    file_close(CALL_PO);

}

