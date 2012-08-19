#ifndef MODULES_NET_H
#define MODULES_NET_H

struct net_driver
{

    struct modules_driver base;
    char mac[6];
    unsigned int (*read)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer);

};

struct net_protocol
{

    char *name;

};

struct net_filesystem
{

    struct modules_filesystem base;
    struct net_driver *drivers[8];
    unsigned int driversCount;
    struct net_protocol *protocols[16];
    unsigned int protocolsCount;
    void (*register_driver)(struct net_filesystem *self, struct net_driver *driver, unsigned int (*read)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer));
    void (*register_protocol)(struct net_filesystem *self, struct net_protocol *protocol, char *name);
    void (*unregister_driver)(struct net_filesystem *self, struct net_driver *driver);
    void (*unregister_protocol)(struct net_filesystem *self, struct net_protocol *protocol);

};

void net_register_driver(struct net_driver *driver, unsigned int (*read)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer));
void net_register_protocol(struct net_protocol *protocol, char *name);
void net_unregister_driver(struct net_driver *driver);
void net_unregister_protocol(struct net_protocol *protocol);
void net_filesystem_init(struct net_filesystem *filesystem);

#endif

