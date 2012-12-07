struct dev_filesystem
{

    struct vfs_interface base;
    struct dev_interface *interfaces[8];
    unsigned int interfacesCount;
    struct dev_protocol *protocols[16];
    unsigned int protocolsCount;

};

void dev_init_filesystem(struct dev_filesystem *filesystem);
