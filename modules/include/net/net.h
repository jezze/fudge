#ifndef MODULES_NET_H
#define MODULES_NET_H

struct net_interface
{

    struct modules_driver *driver;
    char mac[6];
    unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct net_protocol
{

    char *name;

};

struct net_filesystem
{

    struct vfs_filesystem base;
    struct net_interface *interfaces[8];
    unsigned int interfacesCount;
    struct net_protocol *protocols[16];
    unsigned int protocolsCount;
    void (*register_interface)(struct net_filesystem *self, struct net_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer));
    void (*register_protocol)(struct net_filesystem *self, struct net_protocol *protocol, char *name);
    void (*unregister_interface)(struct net_filesystem *self, struct net_interface *interface);
    void (*unregister_protocol)(struct net_filesystem *self, struct net_protocol *protocol);

};

void net_register_interface(struct net_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer));
void net_register_protocol(struct net_protocol *protocol, char *name);
void net_unregister_interface(struct net_interface *interface);
void net_unregister_protocol(struct net_protocol *protocol);
void net_filesystem_init(struct net_filesystem *filesystem);

#endif

