struct unitsrv
{

    struct server server;
    struct service service;
    unsigned int type;

};

void unitsrv_register(struct unitsrv *unitsrv);
void unitsrv_unregister(struct unitsrv *unitsrv);
void unitsrv_init(struct unitsrv *unitsrv, unsigned int type, char *name);
