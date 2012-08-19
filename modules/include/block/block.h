#ifndef MODULES_BLOCK_H
#define MODULES_BLOCK_H

struct block_driver
{

    struct modules_driver base;
    unsigned int (*read)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer);

};

struct block_protocol
{

    char *name;

};

struct block_filesystem
{

    struct modules_filesystem base;
    struct block_driver *drivers[8];
    unsigned int driversCount;
    struct block_protocol *protocols[16];
    unsigned int protocolsCount;
    void (*register_driver)(struct block_filesystem *self, struct block_driver *driver, unsigned int (*read)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer));
    void (*register_protocol)(struct block_filesystem *self, struct block_protocol *protocol, char *name);
    void (*unregister_driver)(struct block_filesystem *self, struct block_driver *driver);
    void (*unregister_protocol)(struct block_filesystem *self, struct block_protocol *protocol);

};

void block_register_driver(struct block_driver *driver, unsigned int (*read)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer));
void block_register_protocol(struct block_protocol *protocol, char *name);
void block_unregister_driver(struct block_driver *driver);
void block_unregister_protocol(struct block_protocol *protocol);
void block_driver_init(struct block_driver *driver);
void block_filesystem_init(struct block_filesystem *filesystem);

#endif

