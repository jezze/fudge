#ifndef MODULES_NET_H
#define MODULES_NET_H

struct net_interface
{

    struct base_driver *driver;
    char mac[6];
    unsigned int (*read_data)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct net_protocol
{

    char *name;
    unsigned char type[2];
    unsigned int (*read)(struct net_protocol *self, struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct net_protocol *self, struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer);

};

struct net_filesystem
{

    struct vfs_interface base;
    struct net_interface *interfaces[8];
    unsigned int interfacesCount;
    struct net_protocol *protocols[16];
    unsigned int protocolsCount;

};

void net_register_interface(struct net_interface *interface, struct base_driver *driver);
void net_register_protocol(struct net_protocol *protocol);
void net_unregister_interface(struct net_interface *interface);
void net_unregister_protocol(struct net_protocol *protocol);
void net_filesystem_init(struct net_filesystem *filesystem);

#endif

