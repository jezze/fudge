#include <abi.h>
#include <fudge.h>
#include <common.h>

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

    file_open(FILE_PI);
    file_readall(FILE_PI, &mbr, 512);
    file_close(FILE_PI);

    if (mbr.signature[0] != 0x55 || mbr.signature[1] != 0xAA)
        return;

    file_open(FILE_PO);

    for (i = 0; i < 4; i++)
    {

        unsigned int start = (mbr.partition[i].sectorlba[3] << 24) | (mbr.partition[i].sectorlba[2] << 16) | (mbr.partition[i].sectorlba[1] << 8) | (mbr.partition[i].sectorlba[0]);
        unsigned int sectors = (mbr.partition[i].sectortotal[3] << 24) | (mbr.partition[i].sectortotal[2] << 16) | (mbr.partition[i].sectortotal[1] << 8) | (mbr.partition[i].sectortotal[0]);
        unsigned int cstart = mbr.partition[i].cylinderbase | ((mbr.partition[i].sectorbase & 0xC0) << 8);
        unsigned int cend = mbr.partition[i].cylinderlimit | ((mbr.partition[i].sectorlimit & 0xC0) << 8);
        unsigned int hstart = mbr.partition[i].headbase;
        unsigned int hend = mbr.partition[i].headlimit;
        unsigned int sstart = mbr.partition[i].sectorbase & 0x2F;
        unsigned int send = mbr.partition[i].sectorlimit & 0x2F;
        char data[64];
        unsigned int count;

        write_keydec(FILE_PO, "Partition", i);

        if (!mbr.partition[i].systemid)
            continue;

        write_keyhex(FILE_PO, "    Boot", mbr.partition[i].boot, 2);
        write_keyhex(FILE_PO, "    Id", mbr.partition[i].systemid, 2);
        write_keydec(FILE_PO, "    Start", start);
        write_keydec(FILE_PO, "    End", start + sectors - 1);
        write_keydec(FILE_PO, "    Sectors", sectors);

        count = 0;
        count += ascii_wvalue(data, 64, cstart, 10, count);
        count += memory_write(data, 64, "/", 1, count);
        count += ascii_wvalue(data, 64, hstart, 10, count);
        count += memory_write(data, 64, "/", 1, count);
        count += ascii_wvalue(data, 64, sstart, 10, count);

        write_keyvalue(FILE_PO, "    Start-C/H/S", data, count);

        count = 0;
        count += ascii_wvalue(data, 64, cend, 10, count);
        count += memory_write(data, 64, "/", 1, count);
        count += ascii_wvalue(data, 64, hend, 10, count);
        count += memory_write(data, 64, "/", 1, count);
        count += ascii_wvalue(data, 64, send, 10, count);

        write_keyvalue(FILE_PO, "    End-C/H/S", data, count);

    }

    file_close(FILE_PO);

}

