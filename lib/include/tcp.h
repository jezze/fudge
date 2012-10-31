struct tcp_header
{

    unsigned short sp;
    unsigned short tp;
    unsigned int seq;
    unsigned int ack;
    unsigned short flags;
    unsigned short window;
    unsigned short checksum;
    unsigned short urgent;

} __attribute__((packed));
