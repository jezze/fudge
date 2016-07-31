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

void main(void)
{

    struct mbr mbr;
    unsigned int i;

    call_open(CALL_PI);
    file_readall(CALL_PI, &mbr, 512);
    call_close(CALL_PI);

    if (mbr.signature[0] != 0x55 || mbr.signature[1] != 0xAA)
        return;

    call_open(CALL_PO);
    file_writeall(CALL_PO, "Id: ", 4);
    file_writeall(CALL_PO, &mbr.id, 4);
    file_writeall(CALL_PO, "\n", 1);

    for (i = 0; i < 4; i++)
    {

        char num[32];
        unsigned int count;

        file_writeall(CALL_PO, "Partition ", 10);

        count = ascii_wvalue(num, 32, i, 10, 0);

        file_writeall(CALL_PO, num, count);
        file_writeall(CALL_PO, ":\n", 2);

        if (mbr.partition[i].systemid == 0)
            continue;

        file_writeall(CALL_PO, "    Boot 0x", 11);
        count = ascii_wvalue(num, 32, mbr.partition[i].boot, 16, 0);
        file_writeall(CALL_PO, num, count);
        file_writeall(CALL_PO, "\n", 1);
        file_writeall(CALL_PO, "    Systemid 0x", 15);
        count = ascii_wvalue(num, 32, mbr.partition[i].systemid, 16, 0);
        file_writeall(CALL_PO, num, count);
        file_writeall(CALL_PO, "\n", 1);

    }

    call_close(CALL_PO);

}

