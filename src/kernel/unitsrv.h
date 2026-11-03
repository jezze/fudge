struct unitsrv
{

    struct resource resource;
    struct resource_operands operands;
    unsigned int type;

};

void unitsrv_init(struct unitsrv *unitsrv, unsigned int type);
