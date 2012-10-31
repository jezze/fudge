struct icmpv6_header
{

    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned int data;

} __attribute__((packed));
