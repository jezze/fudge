#define SOCKET_MTUSIZE                  1518

struct socket_tcp
{

    unsigned int state;
    unsigned int seq;
    unsigned int ack;
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

unsigned int socket_handle_arp(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int count, void *buffer);
unsigned int socket_handle_icmp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer);
unsigned int socket_handle_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_handle_udp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output);
unsigned int socket_send_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata);
unsigned int socket_send_udp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata);
struct socket *socket_accept_arp(struct socket *local, struct socket *remotes, unsigned int nremotes, unsigned int count, void *buffer);
struct socket *socket_accept_tcp(struct socket *local, struct socket *remotes, unsigned int nremotes, unsigned int count, void *buffer);
struct socket *socket_accept_udp(struct socket *local, struct socket *remotes, unsigned int nremotes, unsigned int count, void *buffer);
unsigned int socket_receive(unsigned int descriptor, struct socket *local, struct socket *remotes, unsigned int nremotes, struct socket *router, void *buffer, unsigned int count);
void socket_listen_tcp(unsigned int descriptor, struct socket *local, struct socket *remotes, unsigned int nremotes, struct socket *router);
void socket_connect_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router);
void socket_resolveremote(unsigned int descriptor, struct socket *local, struct socket *remote);
void socket_resolvelocal(unsigned int descriptor, struct socket *socket);
void socket_bind_ipv4(struct socket *socket, unsigned char address[IPV4_ADDRSIZE]);
void socket_bind_ipv4s(struct socket *socket, char *address);
void socket_bind_tcp(struct socket *socket, unsigned char port[TCP_PORTSIZE], unsigned int seq, unsigned int ack);
void socket_bind_tcps(struct socket *socket, char *port, unsigned int seq, unsigned int ack);
void socket_bind_tcpv(struct socket *socket, unsigned short port, unsigned int seq, unsigned int ack);
void socket_bind_udp(struct socket *socket, unsigned char port[UDP_PORTSIZE]);
void socket_bind_udps(struct socket *socket, char *port);
void socket_bind_udpv(struct socket *socket, unsigned short port);
void socket_init(struct socket *socket);
