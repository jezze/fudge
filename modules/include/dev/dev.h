struct dev_interface
{

    struct base_driver *driver;

};

struct dev_protocol
{

    char *name;

};

struct dev_filesystem
{

    struct vfs_interface base;
    struct dev_interface *interfaces[8];
    unsigned int interfacesCount;
    struct dev_protocol *protocols[16];
    unsigned int protocolsCount;

};

void dev_init_interface(struct dev_interface *interface, struct base_driver *driver);
void dev_init_filesystem(struct dev_filesystem *filesystem);
