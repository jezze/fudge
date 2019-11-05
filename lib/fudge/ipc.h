struct ipc_header
{

    unsigned int type;
    unsigned int source;
    unsigned int length;

};

void ipc_init(struct ipc_header *header, unsigned int type, unsigned int length);
