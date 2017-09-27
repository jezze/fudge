struct core
{

    unsigned int id;
    struct resource resource;
    struct list tasks;

};

void core_init(struct core *core, unsigned int id);
