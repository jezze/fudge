struct icmp_header
{

    unsigned char type;
    unsigned char code;
    unsigned char checksum[2];
    unsigned char data[4];

};
