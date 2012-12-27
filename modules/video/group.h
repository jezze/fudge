struct video_group
{

    struct system_group base;
    struct video_interface *interfaces;
    struct video_protocol *protocols;

};

struct video_filesystem
{

    struct vfs_interface base;
    struct video_interface *interfaces[8];
    unsigned int interfacesCount;
    struct video_protocol *protocols[16];
    unsigned int protocolsCount;

};

void video_init_group(struct video_group *group);
void video_init_filesystem(struct video_filesystem *filesystem);
