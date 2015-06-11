struct ethernet_header
{

    unsigned char tha[6];
    unsigned char sha[6];
    unsigned char type[2];

};

struct ethernet_footer
{

    unsigned char crc[4];

};
