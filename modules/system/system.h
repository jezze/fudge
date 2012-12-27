struct system_group
{

    char *name;
    struct system_group *next;

};

void system_register_group(struct system_group *group);
void system_unregister_group(struct system_group *group);
void system_init_group(struct system_group *group, char *name);
