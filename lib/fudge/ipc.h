struct ipc_header
{

    unsigned int type;
    unsigned int source;
    unsigned int target;
    unsigned int length;
    unsigned int nroutes;
    unsigned int routes[16];

};

unsigned int ipc_addroute(struct ipc_header *header, unsigned int target);
void ipc_create(struct ipc_header *header, unsigned int type, unsigned int length);
