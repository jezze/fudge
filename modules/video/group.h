struct video_group
{

    struct system_group base;
    struct video_interface *interfaces;
    struct video_protocol *protocols;

};

void video_init_group(struct video_group *group);
