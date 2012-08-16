#ifndef MODULES_NET_H
#define MODULES_NET_H

#define NET_DRIVER_TYPE 0x1327

struct block_interface
{

    struct modules_base *module;
    unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct block_protocol
{

    char *name;

};

struct block_driver
{

    struct modules_driver base;
    struct block_interface *interfaces[8];
    unsigned int interfacesCount;
    struct block_protocol *protocols[16];
    unsigned int protocolsCount;
    void (*register_interface)(struct block_driver *self, struct block_interface *interface, struct modules_base *module, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer));
    void (*register_protocol)(struct block_driver *self, struct block_protocol *protocol, char *name);
    void (*unregister_interface)(struct block_driver *self, struct block_interface *interface);
    void (*unregister_protocol)(struct block_driver *self, struct block_protocol *protocol);

};

void block_register_interface(struct block_interface *interface, struct modules_base *module, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer));
void block_register_protocol(struct block_protocol *protocol, char *name);
void block_unregister_interface(struct block_interface *interface);
void block_unregister_protocol(struct block_protocol *protocol);
void block_driver_init(struct block_driver *driver);
void block_filesystem_init(struct modules_filesystem *filesystem, struct modules_driver *driver);

#endif

