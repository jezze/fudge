#define BINARY_RESOURCE_PROTOCOL        2002

struct binary_protocol
{

    struct resource resource;
    unsigned int (*match)(struct vfs_channel *channel, unsigned int id);
    unsigned long (*findsymbol)(struct vfs_channel *channel, unsigned int id, unsigned int count, const char *symbol);
    unsigned long (*copyprogram)(struct vfs_channel *channel, unsigned int id);
    unsigned int (*relocate)(struct vfs_channel *channel, unsigned int id, unsigned int address);

};

struct binary_protocol *binary_findprotocol(struct vfs_channel *channel, unsigned int id);
void binary_initprotocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_channel *channel, unsigned int id), unsigned long (*findsymbol)(struct vfs_channel *channel, unsigned int id, unsigned int count, const char *symbol), unsigned long (*copyprogram)(struct vfs_channel *channel, unsigned int id), unsigned int (*relocate)(struct vfs_channel *channel, unsigned int id, unsigned int address));
void binary_setupelf();
