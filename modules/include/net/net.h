#ifndef MODULES_NET_H
#define MODULES_NET_H

#define NET_DRIVER_TYPE 0x1327

struct net_interface
{

    struct modules_base *module;
    unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct net_protocol
{

    char *name;

};

struct net_driver
{

    struct modules_driver base;
    struct net_interface *interfaces[8];
    unsigned int interfacesCount;
    struct net_protocol *protocols[16];
    unsigned int protocolsCount;
    void (*register_interface)(struct net_driver *self, struct net_interface *interface, struct modules_base *module, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer));
    void (*register_protocol)(struct net_driver *self, struct net_protocol *protocol, char *name);
    void (*unregister_interface)(struct net_driver *self, struct net_interface *interface);
    void (*unregister_protocol)(struct net_driver *self, struct net_protocol *protocol);

};

void net_register_interface(struct net_interface *interface, struct modules_base *module, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer));
void net_register_protocol(struct net_protocol *protocol, char *name);
void net_unregister_interface(struct net_interface *interface);
void net_unregister_protocol(struct net_protocol *protocol);
void net_driver_init(struct net_driver *driver);
void net_filesystem_init(struct modules_filesystem *filesystem, struct modules_driver *driver);

#endif

