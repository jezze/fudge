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
    unsigned char address[IPV4_ADDRSIZE];
    union { struct socket_tcp tcp; struct socket_udp udp; } info;

};

void socket_listen(struct socket *local);
void socket_connect(struct socket *local, struct socket *remote);
void socket_init(struct socket *socket, unsigned char protocol, unsigned char address[IPV4_ADDRSIZE]);
void socket_tcp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[TCP_PORTSIZE], unsigned int seq);
void socket_udp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[UDP_PORTSIZE]);
