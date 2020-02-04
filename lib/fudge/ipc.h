struct ipc_header
{

    unsigned int type;
    unsigned int source;
    unsigned int length;

};

unsigned int ipc_datasize(struct ipc_header *header);
void ipc_init(struct ipc_header *header, unsigned int type, unsigned int length);
