#ifndef MODULES_TCP_H
#define MODULES_TCP_H

struct tcp_protocol
{

    struct net_protocol base;
    unsigned char ip[4];

};

void tcp_protocol_init(struct tcp_protocol *protocol);

#endif

