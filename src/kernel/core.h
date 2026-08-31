struct core
{

    struct resource resource;
    struct list tasks;
    unsigned int itask;

};

void core_register(struct core *core);
void core_unregister(struct core *core);
void core_init(struct core *core);
