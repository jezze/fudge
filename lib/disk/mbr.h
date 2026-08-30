struct mbr_partition
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
    struct mbr_partition partition[4];
    unsigned char signature[2];

};

unsigned int mbr_validate(struct mbr *mbr);
