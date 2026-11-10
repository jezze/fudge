#define DHCP_MAGIC                      0x63825363
#define DHCP_OP_REQUEST                 1
#define DHCP_OP_REPLY                   2
#define DHCP_FLAGS_BROADCAST            0x8000
#define DHCP_MINSIZE                    300
#define DHCP_DISCOVER                   1
#define DHCP_OFFER                      2
#define DHCP_REQUEST                    3
#define DHCP_ACK                        5
#define DHCP_NAK                        6
#define DHCP_OPTION_PAD                 0
#define DHCP_OPTION_NETMASK             1
#define DHCP_OPTION_ROUTER              3
#define DHCP_OPTION_DNS                 6
#define DHCP_OPTION_REQUESTEDADDRESS    50
#define DHCP_OPTION_TYPE                53
#define DHCP_OPTION_SERVER              54
#define DHCP_OPTION_PARAMETERS          55
#define DHCP_OPTION_END                 255
#define DHCP_CLIENTPORT                 68
#define DHCP_SERVERPORT                 67

struct dhcp_header
{

    unsigned char op[1];
    unsigned char htype[1];
    unsigned char hlen[1];
    unsigned char hops[1];
    unsigned char xid[4];
    unsigned char secs[2];
    unsigned char flags[2];
    unsigned char ciaddr[4];
    unsigned char yiaddr[4];
    unsigned char siaddr[4];
    unsigned char giaddr[4];
    unsigned char chaddr[16];
    unsigned char sname[64];
    unsigned char file[128];
    unsigned char magic[4];

};
