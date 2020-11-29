struct socket_tcp
{

    unsigned int state;
    unsigned int seq;
    unsigned char port[TCP_PORTSIZE];

};

struct socket_udp
{

    unsigned char port[UDP_PORTSIZE];

};

struct socket
{

    unsigned int active;
    unsigned char protocol;
    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned char paddress[IPV4_ADDRSIZE];
    union { struct socket_tcp tcp; struct socket_udp udp; } info;

};

unsigned short socket_load16(unsigned char seq[2]);
unsigned int socket_load32(unsigned char seq[4]);
void socket_save16(unsigned char seq[2], unsigned short num);
void socket_save32(unsigned char seq[4], unsigned int num);
struct arp_header *socket_arp_create(struct socket *local, struct socket *remote, void *buffer, unsigned short operation);
struct ethernet_header *socket_ethernet_create(struct socket *local, struct socket *remote, void *buffer);
struct ipv4_header *socket_ipv4_create(struct socket *local, struct socket *remote, void *buffer, unsigned char protocol, unsigned short length);
struct tcp_header *socket_tcp_create(struct socket *local, struct socket *remote, void *buffer, unsigned short flags, unsigned int seq, unsigned int ack);
struct udp_header *socket_udp_create(struct socket *local, struct socket *remote, void *buffer, unsigned int count);
void socket_listen(struct socket *local);
void socket_connect(struct socket *local, struct socket *remote);
void socket_init(struct socket *socket, unsigned char protocol, unsigned char address[IPV4_ADDRSIZE]);
void socket_tcp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[TCP_PORTSIZE], unsigned int seq);
void socket_udp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[UDP_PORTSIZE]);
