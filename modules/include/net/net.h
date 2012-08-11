#ifndef MODULES_NET_H
#define MODULES_NET_H

#define NET_DRIVER_TYPE 0x1327

struct net_interface
{

    unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct net_driver
{

    struct modules_driver base;
    struct net_interface *interfaces[8];
    unsigned int interfacesCount;
    void (*register_interface)(struct net_driver *self, struct net_interface *interface, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer));
    void (*unregister_interface)(struct net_driver *self, struct net_interface *interface);

};

void net_driver_init(struct net_driver *driver);

#endif

