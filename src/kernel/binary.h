struct binary_protocol
{

    struct resource resource;
    unsigned int (*match)(struct vfs_channel *channel, unsigned int id);
    unsigned long (*findsymbol)(struct vfs_channel *channel, unsigned int id, unsigned int count, char *symbol);
    unsigned long (*findentry)(struct vfs_channel *channel, unsigned int id);
    unsigned long (*findbase)(struct vfs_channel *channel, unsigned int id, unsigned long address);
    void (*copyprogram)(struct vfs_channel *channel, unsigned int id);
    unsigned int (*relocate)(struct vfs_channel *channel, unsigned int id, unsigned int address);

};

struct binary_protocol *binary_findprotocol(struct vfs_channel *channel, unsigned int id);
void binary_initprotocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_channel *channel, unsigned int id), unsigned long (*findsymbol)(struct vfs_channel *channel, unsigned int id, unsigned int count, char *symbol), unsigned long (*findentry)(struct vfs_channel *channel, unsigned int id), unsigned long (*findbase)(struct vfs_channel *channel, unsigned int id, unsigned long address), void (*copyprogram)(struct vfs_channel *channel, unsigned int id), unsigned int (*relocate)(struct vfs_channel *channel, unsigned int id, unsigned int address));
void binary_setupelf(void);
