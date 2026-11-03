struct dirsrv
{

    struct resource resource;
    struct resource_operands operands;
    unsigned int type;

};

void dirsrv_init(struct dirsrv *dirsrv, unsigned int type);
