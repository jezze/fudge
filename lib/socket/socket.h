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
    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned char paddress[IPV4_ADDRSIZE];
    union { struct socket_tcp tcp; struct socket_udp udp; } info;

};

void socket_loadarplocal(unsigned int descriptor, struct socket *local);
unsigned int socket_ethernet_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_arp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_ipv4_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_icmp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_tcp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_udp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_arp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct arp_header *header, unsigned char *pdata);
unsigned int socket_icmp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct icmp_header *header, void *pdata, unsigned int psize);
unsigned int socket_tcp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct tcp_header *header, void *pdata, unsigned int psize);
unsigned int socket_udp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct udp_header *header, void *pdata, unsigned int psize);
unsigned int socket_receive(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_send(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote, unsigned int psize, void *pdata);
void socket_listen(unsigned int descriptor, unsigned char protocol, struct socket *local);
void socket_connect(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote);
void socket_resolve(unsigned int descriptor, struct socket *local, struct socket *remote);
void socket_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE]);
void socket_tcp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[TCP_PORTSIZE], unsigned int seq);
void socket_udp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[UDP_PORTSIZE]);
