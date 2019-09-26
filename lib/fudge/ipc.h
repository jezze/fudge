struct ipc_route
{

    unsigned int target;
    unsigned int session;

};

struct ipc_header
{

    unsigned int type;
    unsigned int source;
    unsigned int target;
    unsigned int session;
    unsigned int length;
    unsigned int nroutes;
    struct ipc_route routes[16];

};

unsigned int ipc_addroute(struct ipc_header *header, unsigned int target, unsigned int session);
void ipc_create(struct ipc_header *header, unsigned int type, unsigned int length);
