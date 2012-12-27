struct kbd_group
{

    struct system_group base;
    struct kbd_interface *interfaces;
    struct kbd_protocol *protocols;

};

void kbd_init_group(struct kbd_group *group);
