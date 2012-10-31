struct icmpv4_header
{

    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned int data;

} __attribute__((packed));
