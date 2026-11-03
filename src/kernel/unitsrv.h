struct unitsrv
{

    struct server server;
    unsigned int type;

};

void unitsrv_init(struct unitsrv *unitsrv, unsigned int type);
