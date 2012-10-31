struct ethernet_header
{

    unsigned char tha[6];
    unsigned char sha[6];
    unsigned char type[2];

} __attribute__((packed));

struct ethernet_footer
{

    unsigned char crc[4];

};

struct ethernet_interface
{

    unsigned char mac[6];
    unsigned char ip[4];

} __attribute__((packed));
