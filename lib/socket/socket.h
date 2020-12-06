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

    unsigned int resolved;
    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned char paddress[IPV4_ADDRSIZE];
    union { struct socket_tcp tcp; struct socket_udp udp; } info;

};

unsigned int socket_arp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned short operation, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char sip[IPV4_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE]);
unsigned int socket_arp_build2(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned short operation, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char sip[IPV4_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE]);
unsigned int socket_icmp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned char type, unsigned char code, unsigned int count, void *buffer);
unsigned int socket_tcp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned short flags, unsigned int seq, unsigned int ack, unsigned int count, void *buffer);
unsigned int socket_udp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned int count, void *buffer);
unsigned int socket_ethernet_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_arp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_ipv4_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_icmp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_tcp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_udp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_icmp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct icmp_header *header, void *pdata, unsigned int psize);
unsigned int socket_tcp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct tcp_header *header, void *pdata, unsigned int psize);
unsigned int socket_udp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct udp_header *header, void *pdata, unsigned int psize);
unsigned int socket_receive(unsigned int descriptor, unsigned int protocol, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_send(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata);
void socket_listen(unsigned int descriptor, unsigned char protocol, struct socket *local);
void socket_connect(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote, struct socket *router);
void socket_resolveremote(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router);
void socket_resolvelocal(unsigned int descriptor, struct socket *socket);
void socket_bind(struct socket *socket, unsigned char address[IPV4_ADDRSIZE]);
void socket_bind_tcp(struct socket *socket, unsigned char port[TCP_PORTSIZE], unsigned int seq);
void socket_bind_udp(struct socket *socket, unsigned char port[UDP_PORTSIZE]);
void socket_init(struct socket *socket);
