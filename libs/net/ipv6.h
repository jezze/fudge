struct ipv6_header
{

    unsigned char label[4];
    unsigned char length[2];
    unsigned char next;
    unsigned char hop;
    unsigned char sip[8];
    unsigned char tip[8];

};
